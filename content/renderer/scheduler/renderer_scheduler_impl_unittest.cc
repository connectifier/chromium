// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/scheduler/renderer_scheduler_impl.h"

#include "base/callback.h"
#include "cc/output/begin_frame_args.h"
#include "cc/test/ordered_simple_task_runner.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

namespace {
class FakeInputEvent : public blink::WebInputEvent {
 public:
  explicit FakeInputEvent(blink::WebInputEvent::Type event_type)
      : WebInputEvent(sizeof(FakeInputEvent)) {
    type = event_type;
  }

  FakeInputEvent(blink::WebInputEvent::Type event_type, int event_modifiers)
      : WebInputEvent(sizeof(FakeInputEvent)) {
    type = event_type;
    modifiers = event_modifiers;
  }
};
};  // namespace

class RendererSchedulerImplTest : public testing::Test {
 public:
  using Policy = RendererSchedulerImpl::Policy;

  RendererSchedulerImplTest()
      : clock_(cc::TestNowSource::Create(5000)),
        mock_task_runner_(new cc::OrderedSimpleTaskRunner(clock_, false)),
        scheduler_(new RendererSchedulerImpl(mock_task_runner_)),
        default_task_runner_(scheduler_->DefaultTaskRunner()),
        compositor_task_runner_(scheduler_->CompositorTaskRunner()),
        loading_task_runner_(scheduler_->LoadingTaskRunner()),
        idle_task_runner_(scheduler_->IdleTaskRunner()) {
    scheduler_->SetTimeSourceForTesting(clock_);
  }
  ~RendererSchedulerImplTest() override {}

  void RunUntilIdle() { mock_task_runner_->RunUntilIdle(); }

  void DoMainFrame() {
    scheduler_->WillBeginFrame(cc::BeginFrameArgs::Create(
        BEGINFRAME_FROM_HERE, clock_->Now(), base::TimeTicks(),
        base::TimeDelta::FromMilliseconds(1000), cc::BeginFrameArgs::NORMAL));
    scheduler_->DidCommitFrameToCompositor();
  }

  void EnableIdleTasks() { DoMainFrame(); }

  Policy CurrentPolicy() { return scheduler_->current_policy_; }

 protected:
  static base::TimeDelta priority_escalation_after_input_duration() {
    return base::TimeDelta::FromMilliseconds(
        RendererSchedulerImpl::kPriorityEscalationAfterInputMillis);
  }

  scoped_refptr<cc::TestNowSource> clock_;
  scoped_refptr<cc::OrderedSimpleTaskRunner> mock_task_runner_;

  scoped_ptr<RendererSchedulerImpl> scheduler_;
  scoped_refptr<base::SingleThreadTaskRunner> default_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> compositor_task_runner_;
  scoped_refptr<base::SingleThreadTaskRunner> loading_task_runner_;
  scoped_refptr<SingleThreadIdleTaskRunner> idle_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(RendererSchedulerImplTest);
};

void NullTask() {
}

void OrderedTestTask(int value, int* result) {
  *result = (*result << 4) | value;
}

void UnorderedTestTask(int value, int* result) {
  *result += value;
}

void AppendToVectorTestTask(std::vector<std::string>* vector,
                            std::string value) {
  vector->push_back(value);
}

void AppendToVectorIdleTestTask(std::vector<std::string>* vector,
                                std::string value,
                                base::TimeTicks deadline) {
  AppendToVectorTestTask(vector, value);
}

void AppendToVectorReentrantTask(
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    std::vector<int>* vector,
    int* reentrant_count,
    int max_reentrant_count) {
  vector->push_back((*reentrant_count)++);
  if (*reentrant_count < max_reentrant_count) {
    task_runner->PostTask(
        FROM_HERE, base::Bind(AppendToVectorReentrantTask, task_runner, vector,
                              reentrant_count, max_reentrant_count));
  }
}

void IdleTestTask(int* run_count,
                  base::TimeTicks* deadline_out,
                  base::TimeTicks deadline) {
  (*run_count)++;
  *deadline_out = deadline;
}

void RepostingIdleTestTask(
    scoped_refptr<SingleThreadIdleTaskRunner> idle_task_runner,
    int* run_count,
    base::TimeTicks deadline) {
  if (*run_count == 0) {
    idle_task_runner->PostIdleTask(
        FROM_HERE,
        base::Bind(&RepostingIdleTestTask, idle_task_runner, run_count));
  }
  (*run_count)++;
}

void UpdateClockToDeadlineIdleTestTask(
    scoped_refptr<cc::TestNowSource> clock,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    int* run_count,
    base::TimeTicks deadline) {
  clock->SetNow(deadline);
  // Due to the way in which OrderedSimpleTestRunner orders tasks and the fact
  // that we updated the time within a task, the delayed pending task to call
  // EndIdlePeriod will not happen until after a TaskQueueManager DoWork, so
  // post a normal task here to ensure it runs before the next idle task.
  task_runner->PostTask(FROM_HERE, base::Bind(NullTask));
  (*run_count)++;
}

void PostingYieldingTestTask(
    RendererSchedulerImpl* scheduler,
    scoped_refptr<base::SingleThreadTaskRunner> task_runner,
    bool simulate_input,
    bool* should_yield_before,
    bool* should_yield_after) {
  *should_yield_before = scheduler->ShouldYieldForHighPriorityWork();
  task_runner->PostTask(FROM_HERE, base::Bind(NullTask));
  if (simulate_input) {
    scheduler->DidReceiveInputEventOnCompositorThread(
        FakeInputEvent(blink::WebInputEvent::GestureFlingStart));
  }
  *should_yield_after = scheduler->ShouldYieldForHighPriorityWork();
}

void AnticipationTestTask(RendererSchedulerImpl* scheduler,
                          bool simulate_input,
                          bool* is_anticipated_before,
                          bool* is_anticipated_after) {
  *is_anticipated_before = scheduler->IsHighPriorityWorkAnticipated();
  if (simulate_input) {
    scheduler->DidReceiveInputEventOnCompositorThread(
        FakeInputEvent(blink::WebInputEvent::GestureFlingStart));
  }
  *is_anticipated_after = scheduler->IsHighPriorityWorkAnticipated();
}

TEST_F(RendererSchedulerImplTest, TestPostDefaultTask) {
  int result = 0;
  default_task_runner_->PostTask(FROM_HERE,
                                 base::Bind(OrderedTestTask, 1, &result));
  default_task_runner_->PostTask(FROM_HERE,
                                 base::Bind(OrderedTestTask, 2, &result));
  default_task_runner_->PostTask(FROM_HERE,
                                 base::Bind(OrderedTestTask, 3, &result));
  default_task_runner_->PostTask(FROM_HERE,
                                 base::Bind(OrderedTestTask, 4, &result));
  RunUntilIdle();
  EXPECT_EQ(0x1234, result);
}

TEST_F(RendererSchedulerImplTest, TestPostDefaultAndCompositor) {
  int result = 0;
  default_task_runner_->PostTask(FROM_HERE,
                                 base::Bind(&UnorderedTestTask, 1, &result));
  compositor_task_runner_->PostTask(FROM_HERE,
                                    base::Bind(&UnorderedTestTask, 2, &result));
  RunUntilIdle();
  EXPECT_EQ(3, result);
}

TEST_F(RendererSchedulerImplTest, TestRentrantTask) {
  int count = 0;
  std::vector<int> order;
  default_task_runner_->PostTask(
      FROM_HERE, base::Bind(AppendToVectorReentrantTask, default_task_runner_,
                            &order, &count, 5));
  RunUntilIdle();

  EXPECT_THAT(order, testing::ElementsAre(0, 1, 2, 3, 4));
}

TEST_F(RendererSchedulerImplTest, TestPostIdleTask) {
  int run_count = 0;
  base::TimeTicks expected_deadline =
      clock_->Now() + base::TimeDelta::FromMilliseconds(2300);
  base::TimeTicks deadline_in_task;

  clock_->AdvanceNow(base::TimeDelta::FromMilliseconds(100));
  idle_task_runner_->PostIdleTask(
      FROM_HERE, base::Bind(&IdleTestTask, &run_count, &deadline_in_task));

  RunUntilIdle();
  EXPECT_EQ(0, run_count);  // Shouldn't run yet as no WillBeginFrame.

  scheduler_->WillBeginFrame(cc::BeginFrameArgs::Create(
      BEGINFRAME_FROM_HERE, clock_->Now(), base::TimeTicks(),
      base::TimeDelta::FromMilliseconds(1000), cc::BeginFrameArgs::NORMAL));
  RunUntilIdle();
  EXPECT_EQ(0, run_count);  // Shouldn't run as no DidCommitFrameToCompositor.

  clock_->AdvanceNow(base::TimeDelta::FromMilliseconds(1200));
  scheduler_->DidCommitFrameToCompositor();
  RunUntilIdle();
  EXPECT_EQ(0, run_count);  // We missed the deadline.

  scheduler_->WillBeginFrame(cc::BeginFrameArgs::Create(
      BEGINFRAME_FROM_HERE, clock_->Now(), base::TimeTicks(),
      base::TimeDelta::FromMilliseconds(1000), cc::BeginFrameArgs::NORMAL));
  clock_->AdvanceNow(base::TimeDelta::FromMilliseconds(800));
  scheduler_->DidCommitFrameToCompositor();
  RunUntilIdle();
  EXPECT_EQ(1, run_count);
  EXPECT_EQ(expected_deadline, deadline_in_task);
}

TEST_F(RendererSchedulerImplTest, TestRepostingIdleTask) {
  int run_count = 0;

  idle_task_runner_->PostIdleTask(
      FROM_HERE,
      base::Bind(&RepostingIdleTestTask, idle_task_runner_, &run_count));
  EnableIdleTasks();
  RunUntilIdle();
  EXPECT_EQ(1, run_count);

  // Reposted tasks shouldn't run until next idle period.
  RunUntilIdle();
  EXPECT_EQ(1, run_count);

  EnableIdleTasks();
  RunUntilIdle();
  EXPECT_EQ(2, run_count);
}

TEST_F(RendererSchedulerImplTest, TestIdleTaskExceedsDeadline) {
  mock_task_runner_->SetAutoAdvanceNowToPendingTasks(true);
  int run_count = 0;

  // Post two UpdateClockToDeadlineIdleTestTask tasks.
  idle_task_runner_->PostIdleTask(
      FROM_HERE, base::Bind(&UpdateClockToDeadlineIdleTestTask, clock_,
                            default_task_runner_, &run_count));
  idle_task_runner_->PostIdleTask(
      FROM_HERE, base::Bind(&UpdateClockToDeadlineIdleTestTask, clock_,
                            default_task_runner_, &run_count));

  EnableIdleTasks();
  RunUntilIdle();
  // Only the first idle task should execute since it's used up the deadline.
  EXPECT_EQ(1, run_count);

  EnableIdleTasks();
  RunUntilIdle();
  // Second task should be run on the next idle period.
  EXPECT_EQ(2, run_count);
}

TEST_F(RendererSchedulerImplTest, TestPostIdleTaskAfterWakeup) {
  base::TimeTicks deadline_in_task;
  int run_count = 0;

  idle_task_runner_->PostIdleTaskAfterWakeup(
      FROM_HERE, base::Bind(&IdleTestTask, &run_count, &deadline_in_task));

  EnableIdleTasks();
  RunUntilIdle();
  // Shouldn't run yet as no other task woke up the scheduler.
  EXPECT_EQ(0, run_count);

  idle_task_runner_->PostIdleTaskAfterWakeup(
      FROM_HERE, base::Bind(&IdleTestTask, &run_count, &deadline_in_task));

  EnableIdleTasks();
  RunUntilIdle();
  // Another after wakeup idle task shouldn't wake the scheduler.
  EXPECT_EQ(0, run_count);

  default_task_runner_->PostTask(FROM_HERE, base::Bind(&NullTask));

  RunUntilIdle();
  EnableIdleTasks();  // Must start a new idle period before idle task runs.
  RunUntilIdle();
  // Execution of default task queue task should trigger execution of idle task.
  EXPECT_EQ(2, run_count);
}

TEST_F(RendererSchedulerImplTest, TestPostIdleTaskAfterWakeupWhileAwake) {
  base::TimeTicks deadline_in_task;
  int run_count = 0;

  default_task_runner_->PostTask(FROM_HERE, base::Bind(&NullTask));
  idle_task_runner_->PostIdleTaskAfterWakeup(
      FROM_HERE, base::Bind(&IdleTestTask, &run_count, &deadline_in_task));

  RunUntilIdle();
  EnableIdleTasks();  // Must start a new idle period before idle task runs.
  RunUntilIdle();
  // Should run as the scheduler was already awakened by the normal task.
  EXPECT_EQ(1, run_count);
}

TEST_F(RendererSchedulerImplTest, TestPostIdleTaskWakesAfterWakeupIdleTask) {
  base::TimeTicks deadline_in_task;
  int run_count = 0;

  idle_task_runner_->PostIdleTaskAfterWakeup(
      FROM_HERE, base::Bind(&IdleTestTask, &run_count, &deadline_in_task));
  idle_task_runner_->PostIdleTask(
      FROM_HERE, base::Bind(&IdleTestTask, &run_count, &deadline_in_task));

  EnableIdleTasks();
  RunUntilIdle();
  // Must start a new idle period before after-wakeup idle task runs.
  EnableIdleTasks();
  RunUntilIdle();
  // Normal idle task should wake up after-wakeup idle task.
  EXPECT_EQ(2, run_count);
}

TEST_F(RendererSchedulerImplTest, TestDelayedEndIdlePeriodCanceled) {
  int run_count = 0;

  base::TimeTicks deadline_in_task;
  idle_task_runner_->PostIdleTask(
      FROM_HERE, base::Bind(&IdleTestTask, &run_count, &deadline_in_task));

  // Trigger the beginning of an idle period for 1000ms.
  scheduler_->WillBeginFrame(cc::BeginFrameArgs::Create(
      BEGINFRAME_FROM_HERE, clock_->Now(), base::TimeTicks(),
      base::TimeDelta::FromMilliseconds(1000), cc::BeginFrameArgs::NORMAL));
  DoMainFrame();

  // End the idle period early (after 500ms), and send a WillBeginFrame which
  // specifies that the next idle period should end 1000ms from now.
  clock_->AdvanceNow(base::TimeDelta::FromMilliseconds(500));
  scheduler_->WillBeginFrame(cc::BeginFrameArgs::Create(
      BEGINFRAME_FROM_HERE, clock_->Now(), base::TimeTicks(),
      base::TimeDelta::FromMilliseconds(1000), cc::BeginFrameArgs::NORMAL));

  RunUntilIdle();
  EXPECT_EQ(0, run_count);  // Not currently in an idle period.

  // Trigger the start of the idle period before the task to end the previous
  // idle period has been triggered.
  clock_->AdvanceNow(base::TimeDelta::FromMilliseconds(400));
  scheduler_->DidCommitFrameToCompositor();

  // Post a task which simulates running until after the previous end idle
  // period delayed task was scheduled for
  scheduler_->DefaultTaskRunner()->PostTask(FROM_HERE, base::Bind(NullTask));
  clock_->AdvanceNow(base::TimeDelta::FromMilliseconds(300));

  RunUntilIdle();
  EXPECT_EQ(1, run_count);  // We should still be in the new idle period.
}

TEST_F(RendererSchedulerImplTest, TestDefaultPolicy) {
  std::vector<std::string> order;

  loading_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("L1")));
  idle_task_runner_->PostIdleTask(
      FROM_HERE,
      base::Bind(&AppendToVectorIdleTestTask, &order, std::string("I1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  EnableIdleTasks();
  RunUntilIdle();
  EXPECT_THAT(order, testing::ElementsAre(
      std::string("L1"), std::string("D1"), std::string("C1"),
      std::string("D2"), std::string("C2"), std::string("I1")));
}

TEST_F(RendererSchedulerImplTest, TestCompositorPolicy) {
  std::vector<std::string> order;

  loading_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("L1")));
  idle_task_runner_->PostIdleTask(
      FROM_HERE,
      base::Bind(&AppendToVectorIdleTestTask, &order, std::string("I1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureFlingStart));
  EnableIdleTasks();
  RunUntilIdle();
  EXPECT_THAT(order, testing::ElementsAre(
      std::string("C1"), std::string("C2"), std::string("D1"),
      std::string("D2"), std::string("L1"), std::string("I1")));
}

TEST_F(RendererSchedulerImplTest, TestCompositorPolicy_DidAnimateForInput) {
  std::vector<std::string> order;

  idle_task_runner_->PostIdleTask(
      FROM_HERE,
      base::Bind(&AppendToVectorIdleTestTask, &order, std::string("I1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidAnimateForInputOnCompositorThread();
  EnableIdleTasks();
  RunUntilIdle();
  EXPECT_THAT(order, testing::ElementsAre(std::string("C1"), std::string("C2"),
                                          std::string("D1"), std::string("D2"),
                                          std::string("I1")));
}

TEST_F(RendererSchedulerImplTest, TestTouchstartPolicy) {
  std::vector<std::string> order;

  loading_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("L1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  // Observation of touchstart should defer execution of idle and loading tasks.
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::TouchStart));
  RunUntilIdle();
  EXPECT_THAT(order,
              testing::ElementsAre(std::string("C1"), std::string("C2"),
                                   std::string("D1"), std::string("D2")));

  // Meta events like TapDown/FlingCancel shouldn't affect the priority.
  order.clear();
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureFlingCancel));
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureTapDown));
  RunUntilIdle();
  EXPECT_TRUE(order.empty());

  // Action events like ScrollBegin will kick us back into compositor priority,
  // allowing servie of the loading and idle queues.
  order.clear();
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureScrollBegin));
  RunUntilIdle();
  EXPECT_THAT(order, testing::ElementsAre(std::string("L1")));
}

TEST_F(RendererSchedulerImplTest,
       DidReceiveInputEventOnCompositorThread_IgnoresMouseMove_WhenMouseUp) {
  std::vector<std::string> order;

  idle_task_runner_->PostIdleTask(
      FROM_HERE,
      base::Bind(&AppendToVectorIdleTestTask, &order, std::string("I1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::MouseMove));
  EnableIdleTasks();
  RunUntilIdle();
  // Note compositor tasks are not prioritized.
  EXPECT_THAT(order, testing::ElementsAre(std::string("D1"), std::string("C1"),
                                          std::string("D2"), std::string("C2"),
                                          std::string("I1")));
}

TEST_F(RendererSchedulerImplTest,
       DidReceiveInputEventOnCompositorThread_MouseMove_WhenMouseDown) {
  std::vector<std::string> order;

  idle_task_runner_->PostIdleTask(
      FROM_HERE,
      base::Bind(&AppendToVectorIdleTestTask, &order, std::string("I1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidReceiveInputEventOnCompositorThread(FakeInputEvent(
      blink::WebInputEvent::MouseMove, blink::WebInputEvent::LeftButtonDown));
  EnableIdleTasks();
  RunUntilIdle();
  // Note compositor tasks are prioritized.
  EXPECT_THAT(order, testing::ElementsAre(std::string("C1"), std::string("C2"),
                                          std::string("D1"), std::string("D2"),
                                          std::string("I1")));
}

TEST_F(RendererSchedulerImplTest,
       DidReceiveInputEventOnCompositorThread_MouseWheel) {
  std::vector<std::string> order;

  idle_task_runner_->PostIdleTask(
      FROM_HERE,
      base::Bind(&AppendToVectorIdleTestTask, &order, std::string("I1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::MouseWheel));
  EnableIdleTasks();
  RunUntilIdle();
  // Note compositor tasks are prioritized.
  EXPECT_THAT(order, testing::ElementsAre(std::string("C1"), std::string("C2"),
                                          std::string("D1"), std::string("D2"),
                                          std::string("I1")));
}

TEST_F(RendererSchedulerImplTest,
       DidReceiveInputEventOnCompositorThread_IgnoresKeyboardEvents) {
  std::vector<std::string> order;

  idle_task_runner_->PostIdleTask(
      FROM_HERE,
      base::Bind(&AppendToVectorIdleTestTask, &order, std::string("I1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::KeyDown));
  EnableIdleTasks();
  RunUntilIdle();
  // Note compositor tasks are not prioritized.
  EXPECT_THAT(order, testing::ElementsAre(std::string("D1"), std::string("C1"),
                                          std::string("D2"), std::string("C2"),
                                          std::string("I1")));
}

TEST_F(RendererSchedulerImplTest,
       TestCompositorPolicyDoesNotStarveDefaultTasks) {
  std::vector<std::string> order;

  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  for (int i = 0; i < 20; i++) {
    compositor_task_runner_->PostTask(FROM_HERE, base::Bind(&NullTask));
  }
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureFlingStart));
  RunUntilIdle();
  // Ensure that the default D1 task gets to run at some point before the final
  // C2 compositor task.
  EXPECT_THAT(order, testing::ElementsAre(std::string("C1"), std::string("D1"),
                                          std::string("C2")));
}

TEST_F(RendererSchedulerImplTest, TestCompositorPolicyEnds) {
  std::vector<std::string> order;

  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureFlingStart));
  DoMainFrame();
  RunUntilIdle();
  EXPECT_THAT(order,
              testing::ElementsAre(std::string("C1"), std::string("C2"),
                                   std::string("D1"), std::string("D2")));

  order.clear();
  clock_->AdvanceNow(base::TimeDelta::FromMilliseconds(1000));

  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  // Compositor policy mode should have ended now that the clock has advanced.
  RunUntilIdle();
  EXPECT_THAT(order,
              testing::ElementsAre(std::string("D1"), std::string("C1"),
                                   std::string("D2"), std::string("C2")));
}

TEST_F(RendererSchedulerImplTest, TestTouchstartPolicyEndsAfterTimeout) {
  std::vector<std::string> order;

  loading_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("L1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::TouchStart));
  RunUntilIdle();
  EXPECT_THAT(order,
              testing::ElementsAre(std::string("C1"), std::string("C2"),
                                   std::string("D1"), std::string("D2")));

  order.clear();
  clock_->AdvanceNow(base::TimeDelta::FromMilliseconds(1000));

  // Don't post any compositor tasks to simulate a very long running event
  // handler.
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));

  // Touchstart policy mode should have ended now that the clock has advanced.
  RunUntilIdle();
  EXPECT_THAT(order, testing::ElementsAre(std::string("L1"), std::string("D1"),
                                          std::string("D2")));
}

TEST_F(RendererSchedulerImplTest,
       TestTouchstartPolicyEndsAfterConsecutiveTouchmoves) {
  std::vector<std::string> order;

  loading_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("L1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D1")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C1")));
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("D2")));
  compositor_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AppendToVectorTestTask, &order, std::string("C2")));

  // Observation of touchstart should defer execution of idle and loading tasks.
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::TouchStart));
  DoMainFrame();
  RunUntilIdle();
  EXPECT_THAT(order,
              testing::ElementsAre(std::string("C1"), std::string("C2"),
                                   std::string("D1"), std::string("D2")));

  // Receiving the first touchmove will not affect scheduler priority.
  order.clear();
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::TouchMove));
  DoMainFrame();
  RunUntilIdle();
  EXPECT_TRUE(order.empty());

  // Receiving the second touchmove will kick us back into compositor priority.
  order.clear();
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::TouchMove));
  RunUntilIdle();
  EXPECT_THAT(order, testing::ElementsAre(std::string("L1")));
}

TEST_F(RendererSchedulerImplTest, TestIsHighPriorityWorkAnticipated) {
  bool is_anticipated_before = false;
  bool is_anticipated_after = false;

  bool simulate_input = false;
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AnticipationTestTask, scheduler_.get(), simulate_input,
                 &is_anticipated_before, &is_anticipated_after));
  RunUntilIdle();
  // In its default state, without input receipt, the scheduler should indicate
  // that no high-priority is anticipated.
  EXPECT_FALSE(is_anticipated_before);
  EXPECT_FALSE(is_anticipated_after);

  simulate_input = true;
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AnticipationTestTask, scheduler_.get(), simulate_input,
                 &is_anticipated_before, &is_anticipated_after));
  RunUntilIdle();
  // When input is received, the scheduler should indicate that high-priority
  // work is anticipated.
  EXPECT_FALSE(is_anticipated_before);
  EXPECT_TRUE(is_anticipated_after);

  clock_->AdvanceNow(priority_escalation_after_input_duration() * 2);
  simulate_input = false;
  default_task_runner_->PostTask(
      FROM_HERE,
      base::Bind(&AnticipationTestTask, scheduler_.get(), simulate_input,
                 &is_anticipated_before, &is_anticipated_after));
  RunUntilIdle();
  // Without additional input, the scheduler should indicate that high-priority
  // work is no longer anticipated.
  EXPECT_FALSE(is_anticipated_before);
  EXPECT_FALSE(is_anticipated_after);
}

TEST_F(RendererSchedulerImplTest, TestShouldYield) {
  bool should_yield_before = false;
  bool should_yield_after = false;

  default_task_runner_->PostTask(
      FROM_HERE, base::Bind(&PostingYieldingTestTask, scheduler_.get(),
                            default_task_runner_, false, &should_yield_before,
                            &should_yield_after));
  RunUntilIdle();
  // Posting to default runner shouldn't cause yielding.
  EXPECT_FALSE(should_yield_before);
  EXPECT_FALSE(should_yield_after);

  default_task_runner_->PostTask(
      FROM_HERE, base::Bind(&PostingYieldingTestTask, scheduler_.get(),
                            compositor_task_runner_, false,
                            &should_yield_before, &should_yield_after));
  RunUntilIdle();
  // Posting while not in compositor priority shouldn't cause yielding.
  EXPECT_FALSE(should_yield_before);
  EXPECT_FALSE(should_yield_after);

  default_task_runner_->PostTask(
      FROM_HERE, base::Bind(&PostingYieldingTestTask, scheduler_.get(),
                            compositor_task_runner_, true, &should_yield_before,
                            &should_yield_after));
  RunUntilIdle();
  // We should be able to switch to compositor priority mid-task.
  EXPECT_FALSE(should_yield_before);
  EXPECT_TRUE(should_yield_after);

  // Receiving a touchstart should immediately trigger yielding, even if
  // there's no immediately pending work in the compositor queue.
  EXPECT_FALSE(scheduler_->ShouldYieldForHighPriorityWork());
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::TouchStart));
  EXPECT_TRUE(scheduler_->ShouldYieldForHighPriorityWork());
  RunUntilIdle();
}

TEST_F(RendererSchedulerImplTest, SlowInputEvent) {
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());

  // An input event should bump us into input priority.
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureFlingStart));
  RunUntilIdle();
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // Simulate the input event being queued for a very long time. The compositor
  // task we post here represents the enqueued input task.
  clock_->AdvanceNow(priority_escalation_after_input_duration() * 2);
  compositor_task_runner_->PostTask(FROM_HERE, base::Bind(&NullTask));
  RunUntilIdle();

  // Even though we exceeded the input priority escalation period, we should
  // still be in compositor priority since the input remains queued.
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // Simulate the input event triggering a composition. This should start the
  // countdown for going back into normal policy.
  DoMainFrame();
  RunUntilIdle();
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // After the escalation period ends we should go back into normal mode.
  clock_->AdvanceNow(priority_escalation_after_input_duration() * 2);
  RunUntilIdle();
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());
}

TEST_F(RendererSchedulerImplTest, SlowNoOpInputEvent) {
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());

  // An input event should bump us into input priority.
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureFlingStart));
  RunUntilIdle();
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // Simulate the input event being queued for a very long time. The compositor
  // task we post here represents the enqueued input task.
  clock_->AdvanceNow(priority_escalation_after_input_duration() * 2);
  compositor_task_runner_->PostTask(FROM_HERE, base::Bind(&NullTask));
  RunUntilIdle();

  // Even though we exceeded the input priority escalation period, we should
  // still be in compositor priority since the input remains queued.
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // If we let the compositor queue drain, we should fall out of input
  // priority.
  clock_->AdvanceNow(priority_escalation_after_input_duration() * 2);
  RunUntilIdle();
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());
}

TEST_F(RendererSchedulerImplTest, NoOpInputEvent) {
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());

  // An input event should bump us into input priority.
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureFlingStart));
  RunUntilIdle();
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // If nothing else happens after this, we should drop out of compositor
  // priority after the escalation period ends and stop polling.
  clock_->AdvanceNow(priority_escalation_after_input_duration() * 2);
  RunUntilIdle();
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());
  EXPECT_FALSE(mock_task_runner_->HasPendingTasks());
}

TEST_F(RendererSchedulerImplTest, NoOpInputEventExtendsEscalationPeriod) {
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());

  // Simulate one handled input event.
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureScrollBegin));
  RunUntilIdle();
  DoMainFrame();
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // Send a no-op input event in the middle of the escalation period.
  clock_->AdvanceNow(priority_escalation_after_input_duration() / 2);
  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureScrollUpdate));
  RunUntilIdle();
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // The escalation period should have been extended by the new input event.
  clock_->AdvanceNow(3 * priority_escalation_after_input_duration() / 4);
  RunUntilIdle();
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  clock_->AdvanceNow(priority_escalation_after_input_duration() / 2);
  RunUntilIdle();
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());
}

TEST_F(RendererSchedulerImplTest, InputArrivesAfterBeginFrame) {
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());

  cc::BeginFrameArgs args = cc::BeginFrameArgs::Create(
      BEGINFRAME_FROM_HERE, clock_->Now(), base::TimeTicks(),
      base::TimeDelta::FromMilliseconds(1000), cc::BeginFrameArgs::NORMAL);
  clock_->AdvanceNow(priority_escalation_after_input_duration() / 2);

  scheduler_->DidReceiveInputEventOnCompositorThread(
      FakeInputEvent(blink::WebInputEvent::GestureScrollBegin));

  // Simulate a BeginMainFrame task from the past.
  clock_->AdvanceNow(2 * priority_escalation_after_input_duration());
  scheduler_->WillBeginFrame(args);
  scheduler_->DidCommitFrameToCompositor();

  // This task represents the queued-up input event.
  compositor_task_runner_->PostTask(FROM_HERE, base::Bind(&NullTask));

  // Should remain in input priority policy since the input event hasn't been
  // processed yet.
  clock_->AdvanceNow(2 * priority_escalation_after_input_duration());
  RunUntilIdle();
  EXPECT_EQ(Policy::COMPOSITOR_PRIORITY, CurrentPolicy());

  // Process the input event with a new BeginMainFrame.
  DoMainFrame();
  clock_->AdvanceNow(2 * priority_escalation_after_input_duration());
  RunUntilIdle();
  EXPECT_EQ(Policy::NORMAL, CurrentPolicy());
}

}  // namespace content
