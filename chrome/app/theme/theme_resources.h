// TODO(tc): Come up with a way to automate the generation of these
// IDs so they don't collide with other rc files.

// NOTE: If you change an id you can use the following awk script to
// regenerate this file with no holes in the ids:
//   awk '$1 != "#define"{print$0} $1 == "#define" {row++;printf "#define %-48s %d\n", $2, (row+9000)}' < theme_resources.h

#define IDR_BACK                                         9000
#define IDR_BACK_D                                       9001
#define IDR_BACK_H                                       9002
#define IDR_BACK_P                                       9003
#define IDR_CLOSE                                        9004
#define IDR_CLOSE_H                                      9005
#define IDR_CLOSE_P                                      9006
#define IDR_CONTENT_BOTTOM_CENTER                        9007
#define IDR_CONTENT_BOTTOM_LEFT_CORNER                   9008
#define IDR_CONTENT_BOTTOM_RIGHT_CORNER                  9009
#define IDR_CONTENT_LEFT_SIDE                            9010
#define IDR_CONTENT_RIGHT_SIDE                           9011
#define IDR_CONTENT_TOP_CENTER                           9012
#define IDR_CONTENT_TOP_LEFT_CORNER                      9013
#define IDR_CONTENT_TOP_RIGHT_CORNER                     9014
#define IDR_DEFAULT_FAVICON                              9015
#define IDR_DROP                                         9016
#define IDR_DROP_H                                       9017
#define IDR_DROP_P                                       9018
#define IDR_FORWARD                                      9019
#define IDR_FORWARD_D                                    9020
#define IDR_FORWARD_H                                    9021
#define IDR_FORWARD_P                                    9022
#define IDR_GO                                           9023
#define IDR_GO_H                                         9024
#define IDR_GO_P                                         9025
#define IDR_INFO_BUBBLE_CLOSE                            9026
#define IDR_LOCATIONBG                                   9027
#define IDR_MAXIMIZE                                     9028
#define IDR_MAXIMIZE_H                                   9029
#define IDR_MAXIMIZE_P                                   9030
#define IDR_MENUITEM_CENTER_A                            9031
#define IDR_MENUITEM_CENTER_H                            9032
#define IDR_MENUITEM_LEFT_A                              9033
#define IDR_MENUITEM_LEFT_H                              9034
#define IDR_MENUITEM_RIGHT_A                             9035
#define IDR_MENUITEM_RIGHT_H                             9036
#define IDR_MINIMIZE                                     9037
#define IDR_MINIMIZE_H                                   9038
#define IDR_MINIMIZE_P                                   9039
#define IDR_PLUGIN                                       9040
#define IDR_RELOAD                                       9041
#define IDR_RELOAD_H                                     9042
#define IDR_RELOAD_P                                     9043
#define IDR_LOCATION_BAR_SELECTED_KEYWORD_BACKGROUND_C   9044
#define IDR_LOCATION_BAR_SELECTED_KEYWORD_BACKGROUND_L   9045
#define IDR_LOCATION_BAR_SELECTED_KEYWORD_BACKGROUND_R   9046
#define IDR_STAR                                         9047
#define IDR_STAR_D                                       9048
#define IDR_STAR_H                                       9049
#define IDR_STAR_P                                       9050
#define IDR_STARRED                                      9051
#define IDR_STARRED_H                                    9052
#define IDR_STARRED_P                                    9053
#define IDR_TAB_ACTIVE_CENTER                            9054
#define IDR_TAB_ACTIVE_LEFT                              9055
#define IDR_TAB_ACTIVE_RIGHT                             9056
#define IDR_TAB_CLOSE                                    9057
#define IDR_TAB_CLOSE_H                                  9058
#define IDR_TAB_CLOSE_P                                  9059
#define IDR_TAB_INACTIVE_CENTER                          9060
#define IDR_TAB_INACTIVE_LEFT                            9061
#define IDR_TAB_INACTIVE_RIGHT                           9062
#define IDR_THROBBER                                     9063
#define IDR_TOOLBAR_TOGGLE_KNOB                          9064
#define IDR_TOOLBAR_TOGGLE_TRACK                         9065
#define IDR_WINDOW_BOTTOM_CENTER                         9066
#define IDR_WINDOW_BOTTOM_LEFT_CORNER                    9067
#define IDR_WINDOW_BOTTOM_RIGHT_CORNER                   9068
#define IDR_WINDOW_LEFT_SIDE                             9069
#define IDR_WINDOW_RIGHT_SIDE                            9070
#define IDR_WINDOW_TOP_CENTER                            9071
#define IDR_WINDOW_TOP_LEFT_CORNER                       9072
#define IDR_WINDOW_TOP_RIGHT_CORNER                      9073
#define IDR_WINDOW_BOTTOM_CENTER_OTR                     9074
#define IDR_WINDOW_BOTTOM_LEFT_CORNER_OTR                9075
#define IDR_WINDOW_BOTTOM_RIGHT_CORNER_OTR               9076
#define IDR_WINDOW_LEFT_SIDE_OTR                         9077
#define IDR_WINDOW_RIGHT_SIDE_OTR                        9078
#define IDR_WINDOW_TOP_CENTER_OTR                        9079
#define IDR_WINDOW_TOP_LEFT_CORNER_OTR                   9080
#define IDR_WINDOW_TOP_RIGHT_CORNER_OTR                  9081
#define IDR_TAB_INACTIVE_CENTER_V                        9082
#define IDR_TAB_INACTIVE_LEFT_V                          9083
#define IDR_TAB_INACTIVE_RIGHT_V                         9084
#define IDR_RESTORE                                      9085
#define IDR_RESTORE_H                                    9086
#define IDR_RESTORE_P                                    9087
#define IDR_DEWINDOW_BOTTOM_CENTER                       9088
#define IDR_DEWINDOW_BOTTOM_LEFT_CORNER                  9089
#define IDR_DEWINDOW_BOTTOM_RIGHT_CORNER                 9090
#define IDR_DEWINDOW_LEFT_SIDE                           9091
#define IDR_DEWINDOW_RIGHT_SIDE                          9092
#define IDR_DEWINDOW_TOP_CENTER                          9093
#define IDR_DEWINDOW_TOP_LEFT_CORNER                     9094
#define IDR_DEWINDOW_TOP_RIGHT_CORNER                    9095
#define IDR_DEWINDOW_BOTTOM_CENTER_OTR                   9096
#define IDR_DEWINDOW_BOTTOM_LEFT_CORNER_OTR              9097
#define IDR_DEWINDOW_BOTTOM_RIGHT_CORNER_OTR             9098
#define IDR_DEWINDOW_LEFT_SIDE_OTR                       9099
#define IDR_DEWINDOW_RIGHT_SIDE_OTR                      9100
#define IDR_DEWINDOW_TOP_CENTER_OTR                      9101
#define IDR_DEWINDOW_TOP_LEFT_CORNER_OTR                 9102
#define IDR_DEWINDOW_TOP_RIGHT_CORNER_OTR                9103
#define IDR_DOWNLOAD_BUTTON_CENTER_BOTTOM                9104
#define IDR_DOWNLOAD_BUTTON_CENTER_BOTTOM_H              9105
#define IDR_DOWNLOAD_BUTTON_CENTER_BOTTOM_P              9106
#define IDR_DOWNLOAD_BUTTON_CENTER_MIDDLE                9107
#define IDR_DOWNLOAD_BUTTON_CENTER_MIDDLE_H              9108
#define IDR_DOWNLOAD_BUTTON_CENTER_MIDDLE_P              9109
#define IDR_DOWNLOAD_BUTTON_CENTER_TOP                   9110
#define IDR_DOWNLOAD_BUTTON_CENTER_TOP_H                 9111
#define IDR_DOWNLOAD_BUTTON_CENTER_TOP_P                 9112
#define IDR_DOWNLOAD_BUTTON_LEFT_BOTTOM                  9113
#define IDR_DOWNLOAD_BUTTON_LEFT_BOTTOM_H                9114
#define IDR_DOWNLOAD_BUTTON_LEFT_BOTTOM_P                9115
#define IDR_DOWNLOAD_BUTTON_LEFT_MIDDLE                  9116
#define IDR_DOWNLOAD_BUTTON_LEFT_MIDDLE_H                9117
#define IDR_DOWNLOAD_BUTTON_LEFT_MIDDLE_P                9118
#define IDR_DOWNLOAD_BUTTON_LEFT_TOP                     9119
#define IDR_DOWNLOAD_BUTTON_LEFT_TOP_H                   9120
#define IDR_DOWNLOAD_BUTTON_LEFT_TOP_P                   9121
#define IDR_DOWNLOAD_BUTTON_MENU_BOTTOM                  9122
#define IDR_DOWNLOAD_BUTTON_MENU_BOTTOM_H                9123
#define IDR_DOWNLOAD_BUTTON_MENU_BOTTOM_P                9124
#define IDR_DOWNLOAD_BUTTON_MENU_MIDDLE                  9125
#define IDR_DOWNLOAD_BUTTON_MENU_MIDDLE_H                9126
#define IDR_DOWNLOAD_BUTTON_MENU_MIDDLE_P                9127
#define IDR_DOWNLOAD_BUTTON_MENU_TOP                     9128
#define IDR_DOWNLOAD_BUTTON_MENU_TOP_H                   9129
#define IDR_DOWNLOAD_BUTTON_MENU_TOP_P                   9130
#define IDR_DOWNLOAD_BUTTON_RIGHT_BOTTOM                 9131
#define IDR_DOWNLOAD_BUTTON_RIGHT_BOTTOM_H               9132
#define IDR_DOWNLOAD_BUTTON_RIGHT_BOTTOM_P               9133
#define IDR_DOWNLOAD_BUTTON_RIGHT_BOTTOM_NO_DD           9134
#define IDR_DOWNLOAD_BUTTON_RIGHT_MIDDLE                 9135
#define IDR_DOWNLOAD_BUTTON_RIGHT_MIDDLE_H               9136
#define IDR_DOWNLOAD_BUTTON_RIGHT_MIDDLE_P               9137
#define IDR_DOWNLOAD_BUTTON_RIGHT_MIDDLE_NO_DD           9138
#define IDR_DOWNLOAD_BUTTON_RIGHT_TOP                    9139
#define IDR_DOWNLOAD_BUTTON_RIGHT_TOP_H                  9140
#define IDR_DOWNLOAD_BUTTON_RIGHT_TOP_P                  9141
#define IDR_DOWNLOAD_BUTTON_RIGHT_TOP_NO_DD              9142
#define IDR_DOWNLOAD_PROGRESS_BACKGROUND_16              9143
#define IDR_DOWNLOAD_PROGRESS_FOREGROUND_16              9144
#define IDR_DOWNLOAD_PROGRESS_BACKGROUND_32              9145
#define IDR_DOWNLOAD_PROGRESS_FOREGROUND_32              9146
#define IDR_DOWNLOAD_ICON                                9147
#define IDR_COOKIE_ICON                                  9148
#define IDR_SAD_FAVICON                                  9149
#define IDR_TAB_DROP_UP                                  9150
#define IDR_TAB_DROP_DOWN                                9151
#define IDR_CONSTRAINED_BOTTOM_CENTER                    9152
#define IDR_CONSTRAINED_BOTTOM_LEFT_CORNER               9153
#define IDR_CONSTRAINED_BOTTOM_RIGHT_CORNER              9154
#define IDR_CONSTRAINED_LEFT_SIDE                        9155
#define IDR_CONSTRAINED_RIGHT_SIDE                       9156
#define IDR_CONSTRAINED_TOP_CENTER                       9157
#define IDR_CONSTRAINED_TOP_LEFT_CORNER                  9158
#define IDR_CONSTRAINED_TOP_RIGHT_CORNER                 9159
#define IDR_FIND_BOX_BACKGROUND                          9160
#define IDR_FIND_BOX_BACKGROUND_LEFT                     9161
#define IDR_LOCK                                         9162
#define IDR_WARNING                                      9163
#define IDR_BOOKMARK_BAR_RECENTLY_BOOKMARKED_ICON        9164
#define IDR_STOP                                         9165
#define IDR_STOP_H                                       9166
#define IDR_STOP_P                                       9167
#define IDR_FIND_DLG_LEFT_BACKGROUND                     9168
#define IDR_FIND_DLG_RIGHT_BACKGROUND                    9169
#define IDR_FIND_DLG_MIDDLE_BACKGROUND                   9170
#define IDR_APP_TOP_RIGHT                                9171
#define IDR_APP_TOP_CENTER                               9172
#define IDR_APP_TOP_LEFT                                 9173
#define IDR_APP_DROPARROW                                9174
#define IDR_THROBBER_01                                  9175
#define IDR_THROBBER_02                                  9176
#define IDR_THROBBER_03                                  9177
#define IDR_THROBBER_04                                  9178
#define IDR_THROBBER_05                                  9179
#define IDR_THROBBER_06                                  9180
#define IDR_THROBBER_07                                  9181
#define IDR_THROBBER_08                                  9182
#define IDR_THROBBER_09                                  9183
#define IDR_THROBBER_10                                  9184
#define IDR_THROBBER_11                                  9185
#define IDR_THROBBER_12                                  9186
#define IDR_THROBBER_13                                  9187
#define IDR_THROBBER_14                                  9188
#define IDR_THROBBER_15                                  9189
#define IDR_THROBBER_16                                  9190
#define IDR_THROBBER_17                                  9191
#define IDR_THROBBER_18                                  9192
#define IDR_THROBBER_19                                  9193
#define IDR_THROBBER_20                                  9194
#define IDR_THROBBER_21                                  9195
#define IDR_THROBBER_22                                  9196
#define IDR_THROBBER_23                                  9197
#define IDR_THROBBER_24                                  9198
#define IDR_PAGEINFO_GOOD                                9199
#define IDR_PAGEINFO_BAD                                 9200
#define IDR_NEWTAB_BUTTON                                9201
#define IDR_NEWTAB_BUTTON_H                              9202
#define IDR_NEWTAB_BUTTON_P                              9203
#define IDR_ARROW_RIGHT                                  9204
#define IDR_TEXTBUTTON_TOP_LEFT_H                        9205
#define IDR_TEXTBUTTON_TOP_H                             9206
#define IDR_TEXTBUTTON_TOP_RIGHT_H                       9207
#define IDR_TEXTBUTTON_LEFT_H                            9208
#define IDR_TEXTBUTTON_CENTER_H                          9209
#define IDR_TEXTBUTTON_RIGHT_H                           9210
#define IDR_TEXTBUTTON_BOTTOM_LEFT_H                     9211
#define IDR_TEXTBUTTON_BOTTOM_H                          9212
#define IDR_TEXTBUTTON_BOTTOM_RIGHT_H                    9213
#define IDR_TEXTBUTTON_TOP_LEFT_P                        9214
#define IDR_TEXTBUTTON_TOP_P                             9215
#define IDR_TEXTBUTTON_TOP_RIGHT_P                       9216
#define IDR_TEXTBUTTON_LEFT_P                            9217
#define IDR_TEXTBUTTON_CENTER_P                          9218
#define IDR_TEXTBUTTON_RIGHT_P                           9219
#define IDR_TEXTBUTTON_BOTTOM_LEFT_P                     9220
#define IDR_TEXTBUTTON_BOTTOM_P                          9221
#define IDR_TEXTBUTTON_BOTTOM_RIGHT_P                    9222
#define IDR_SAD_TAB                                      9223
#define IDR_FOLDER_OPEN                                  9224
#define IDR_FOLDER_CLOSED                                9225
#define IDR_OTR_ICON                                     9226
#define IDR_TAB_INACTIVE_LEFT_OTR                        9227
#define IDR_TAB_INACTIVE_CENTER_OTR                      9228
#define IDR_TAB_INACTIVE_RIGHT_OTR                       9229
#define IDR_BOOKMARK_BAR_CHEVRONS                        9230
#define IDR_CONSTRAINED_BOTTOM_CENTER_V                  9231
#define IDR_CONSTRAINED_BOTTOM_LEFT_CORNER_V             9232
#define IDR_CONSTRAINED_BOTTOM_RIGHT_CORNER_V            9233
#define IDR_CONSTRAINED_LEFT_SIDE_V                      9234
#define IDR_CONSTRAINED_RIGHT_SIDE_V                     9235
#define IDR_CONSTRAINED_TOP_CENTER_V                     9236
#define IDR_CONSTRAINED_TOP_LEFT_CORNER_V                9237
#define IDR_CONSTRAINED_TOP_RIGHT_CORNER_V               9238
#define IDR_CONTENT_STAR_D                               9239
#define IDR_CONTENT_STAR_OFF                             9240
#define IDR_CONTENT_STAR_ON                              9241
#define IDR_LOCATION_BAR_KEYWORD_HINT_TAB                9242
#define IDR_ABOUT_BACKGROUND                             9243
#define IDR_FINDINPAGE_PREV                              9244
#define IDR_FINDINPAGE_PREV_H                            9245
#define IDR_FINDINPAGE_PREV_P                            9246
#define IDR_FINDINPAGE_NEXT                              9247
#define IDR_FINDINPAGE_NEXT_H                            9248
#define IDR_FINDINPAGE_NEXT_P                            9249
#define IDR_INFOBAR_RESTORE_SESSION                      9250
#define IDR_INFOBAR_SAVE_PASSWORD                        9251
#define IDR_INFOBAR_SSL_WARNING                          9252
#define IDR_INFOBAR_ALT_NAV_URL                          9253
#define IDR_INFOBAR_PLUGIN_INSTALL                       9254
#define IDR_INFO_BUBBLE_CORNER_TOP_LEFT                  9255
#define IDR_INFO_BUBBLE_CORNER_TOP_RIGHT                 9256
#define IDR_INFO_BUBBLE_CORNER_BOTTOM_LEFT               9257
#define IDR_INFO_BUBBLE_CORNER_BOTTOM_RIGHT              9258
#define IDR_WIZARD_ICON                                  9259
#define IDR_MENU_MARKER                                  9260
#define IDR_FROZEN_TAB_ICON                              9261
#define IDR_FROZEN_PLUGIN_ICON                           9262
#define IDR_UPDATE_AVAILABLE                             9263
#define IDR_MENU_PAGE                                    9264
#define IDR_MENU_CHROME                                  9265
#define IDR_ABOUT_BACKGROUND_RTL                         9266
#define IDR_WIZARD_ICON_RTL                              9267
#define IDR_LOCATIONBG_POPUPMODE_LEFT                    9268
#define IDR_LOCATIONBG_POPUPMODE_CENTER                  9269
#define IDR_LOCATIONBG_POPUPMODE_RIGHT                   9270
#define IDR_CLOSE_SA                                     9271
#define IDR_CLOSE_SA_H                                   9272
#define IDR_CLOSE_SA_P                                   9273
#define IDR_HISTORY_SECTION                              9274
#define IDR_DOWNLOADS_SECTION                            9275
#define IDR_DEFAULT_THUMBNAIL                            9276
#define IDR_THROBBER_WAITING                             9277
#define IDR_INFOBAR_PLUGIN_CRASHED                       9278
#define IDR_UPDATE_UPTODATE                              9279
#define IDR_UPDATE_FAIL                                  9280
#define IDR_CLOSE_BAR                                    9281
#define IDR_CLOSE_BAR_H                                  9282
#define IDR_CLOSE_BAR_P                                  9283
#define IDR_HOME                                         9284
#define IDR_HOME_H                                       9285
#define IDR_HOME_P                                       9286
#define IDR_FIND_BOX_BACKGROUND_LEFT_RTL                 9287
#define IDR_INPUT_GOOD                                   9288
#define IDR_INPUT_ALERT                                  9289
#define IDR_HISTORY_FAVICON                              9290
#define IDR_DOWNLOADS_FAVICON                            9291
#define IDR_MENU_PAGE_RTL                                9292
#define IDR_MENU_CHROME_RTL                              9293
#define IDR_DOWNLOAD_ANIMATION_BEGIN                     9294
#define IDR_TAB_HOVER_LEFT                               9295
#define IDR_TAB_HOVER_CENTER                             9296
#define IDR_TAB_HOVER_RIGHT                              9297
#define IDR_FOLDER_CLOSED_RTL                            9298
#define IDR_FOLDER_OPEN_RTL                              9299
#define IDR_BOOKMARK_BAR_FOLDER                          9300
#define IDR_FIND_DLG_LEFT_BB_BACKGROUND                  9301
#define IDR_FIND_DLG_RIGHT_BB_BACKGROUND                 9302
#define IDR_FIND_DLG_MIDDLE_BB_BACKGROUND                9303
#define IDR_THROBBER_LIGHT                               9304
#define IDR_OTR_ICON_STANDALONE                          9305
#define IDR_PRODUCT_LOGO                                 9306
#define IDR_DISTRIBUTOR_LOGO                             9307
#define IDR_DISTRIBUTOR_LOGO_LIGHT                       9308
#define IDR_BOOKMARK_MANAGER_RECENT_ICON                 9309
#define IDR_BOOKMARK_MANAGER_SEARCH_ICON                 9310
