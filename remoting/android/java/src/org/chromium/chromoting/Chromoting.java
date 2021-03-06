// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chromoting;

import android.accounts.Account;
import android.accounts.AccountManager;
import android.accounts.AccountManagerCallback;
import android.accounts.AccountManagerFuture;
import android.accounts.AuthenticatorException;
import android.accounts.OperationCanceledException;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.os.Bundle;
import android.provider.Settings;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.Toast;

import org.chromium.chromoting.jni.JniInterface;

import java.io.IOException;
import java.util.Arrays;

/**
 * The user interface for querying and displaying a user's host list from the directory server. It
 * also requests and renews authentication tokens using the system account manager.
 */
public class Chromoting extends ActionBarActivity implements JniInterface.ConnectionListener,
        AccountManagerCallback<Bundle>, AdapterView.OnItemSelectedListener, HostListLoader.Callback,
        View.OnClickListener {
    /** Only accounts of this type will be selectable for authentication. */
    private static final String ACCOUNT_TYPE = "com.google";

    /** Scopes at which the authentication token we request will be valid. */
    private static final String TOKEN_SCOPE = "oauth2:https://www.googleapis.com/auth/chromoting "
            + "https://www.googleapis.com/auth/googletalk";

    /** Web page to be displayed in the Help screen when launched from this activity. */
    private static final String HELP_URL =
            "http://support.google.com/chrome/?p=mobile_crd_hostslist";

    /** Web page to be displayed when user triggers the hyperlink for setting up hosts. */
    private static final String HOST_SETUP_URL =
            "https://support.google.com/chrome/answer/1649523";

    /** User's account details. */
    private Account mAccount;

    /** List of accounts on the system. */
    private Account[] mAccounts;

    /** SpinnerAdapter used in the action bar for selecting accounts. */
    private AccountsAdapter mAccountsAdapter;

    /** Account auth token. */
    private String mToken;

    /** Helper for fetching the host list. */
    private HostListLoader mHostListLoader;

    /** List of hosts. */
    private HostInfo[] mHosts = new HostInfo[0];

    /** Refresh button. */
    private MenuItem mRefreshButton;

    /** Host list as it appears to the user. */
    private ListView mHostListView;

    /** Progress view shown instead of the host list when the host list is loading. */
    private View mProgressView;

    /** Dialog for reporting connection progress. */
    private ProgressDialog mProgressIndicator;

    /** Object for fetching OAuth2 access tokens from third party authorization servers. */
    private ThirdPartyTokenFetcher mTokenFetcher;

    /**
     * This is set when receiving an authentication error from the HostListLoader. If that occurs,
     * this flag is set and a fresh authentication token is fetched from the AccountsService, and
     * used to request the host list a second time.
     */
    boolean mTriedNewAuthToken;

    /**
     * Flag to track whether a call to AccountManager.getAuthToken() is currently pending.
     * This avoids infinitely-nested calls in case onStart() gets triggered a second time
     * while a token is being fetched.
     */
    private boolean mWaitingForAuthToken = false;

    private ActionBarDrawerToggle mDrawerToggle;

    /** Shows a warning explaining that a Google account is required, then closes the activity. */
    private void showNoAccountsDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(R.string.noaccounts_message);
        builder.setPositiveButton(R.string.noaccounts_add_account,
                new DialogInterface.OnClickListener() {
                    @SuppressLint("InlinedApi")
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        Intent intent = new Intent(Settings.ACTION_ADD_ACCOUNT);
                        intent.putExtra(Settings.EXTRA_ACCOUNT_TYPES,
                                new String[] { ACCOUNT_TYPE });
                        if (intent.resolveActivity(getPackageManager()) != null) {
                            startActivity(intent);
                        }
                        finish();
                    }
                });
        builder.setNegativeButton(R.string.close, new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int id) {
                    finish();
                }
            });
        builder.setOnCancelListener(new DialogInterface.OnCancelListener() {
                @Override
                public void onCancel(DialogInterface dialog) {
                    finish();
                }
            });

        AlertDialog dialog = builder.create();
        dialog.show();
    }

    /** Shows or hides the progress indicator for loading the host list. */
    private void setHostListProgressVisible(boolean visible) {
        mHostListView.setVisibility(visible ? View.GONE : View.VISIBLE);
        mProgressView.setVisibility(visible ? View.VISIBLE : View.GONE);

        // Hiding the host-list does not automatically hide the empty view, so do that here.
        if (visible) {
            mHostListView.getEmptyView().setVisibility(View.GONE);
        }
    }

    /**
     * Called when the activity is first created. Loads the native library and requests an
     * authentication token from the system.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        mTriedNewAuthToken = false;
        mHostListLoader = new HostListLoader();

        // Get ahold of our view widgets.
        mHostListView = (ListView) findViewById(R.id.hostList_chooser);
        mHostListView.setEmptyView(findViewById(R.id.hostList_empty));
        mProgressView = findViewById(R.id.hostList_progress);

        findViewById(R.id.host_setup_link_android).setOnClickListener(this);

        DrawerLayout drawerLayout = (DrawerLayout) findViewById(R.id.drawer_layout);
        mDrawerToggle = new ActionBarDrawerToggle(this, drawerLayout,
                R.string.open_navigation_drawer, R.string.close_navigation_drawer);
        drawerLayout.setDrawerListener(mDrawerToggle);

        ListView navigationMenu = (ListView) findViewById(R.id.navigation_menu);
        String[] navigationMenuItems = new String[] {
            getString(R.string.actionbar_help)
        };
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, R.layout.navigation_list_item,
                navigationMenuItems);
        navigationMenu.setAdapter(adapter);
        navigationMenu.setOnItemClickListener(
                new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position,
                            long id) {
                        HelpActivity.launch(Chromoting.this, HELP_URL);
                    }
                });

        // Make the navigation drawer icon visible in the ActionBar.
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);

        // Bring native components online.
        JniInterface.loadLibrary(this);
    }

    @Override
    protected void onPostCreate(Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        mDrawerToggle.syncState();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        if (mTokenFetcher != null) {
            if (mTokenFetcher.handleTokenFetched(intent)) {
                mTokenFetcher = null;
            }
        }
    }

    /**
     * Called when the activity becomes visible. This happens on initial launch and whenever the
     * user switches to the activity, for example, by using the window-switcher or when coming from
     * the device's lock screen.
     */
    @Override
    public void onStart() {
        super.onStart();

        mAccounts = AccountManager.get(this).getAccountsByType(ACCOUNT_TYPE);
        if (mAccounts.length == 0) {
            showNoAccountsDialog();
            return;
        }

        SharedPreferences prefs = getPreferences(MODE_PRIVATE);
        int index = -1;
        if (prefs.contains("account_name") && prefs.contains("account_type")) {
            mAccount = new Account(prefs.getString("account_name", null),
                    prefs.getString("account_type", null));
            index = Arrays.asList(mAccounts).indexOf(mAccount);
        }
        if (index == -1) {
            // Preference not loaded, or does not correspond to a valid account, so just pick the
            // first account arbitrarily.
            index = 0;
            mAccount = mAccounts[0];
        }

        getSupportActionBar().setTitle(R.string.mode_me2me);

        mAccountsAdapter = new AccountsAdapter(this, mAccounts);
        Spinner accountsSpinner = (Spinner) findViewById(R.id.accounts_spinner);
        accountsSpinner.setAdapter(mAccountsAdapter);
        accountsSpinner.setOnItemSelectedListener(this);
        accountsSpinner.setSelection(index);

        refreshHostList();
    }

    /** Called when the activity is finally finished. */
    @Override
    public void onDestroy() {
        super.onDestroy();
        JniInterface.disconnectFromHost();
    }

    /** Called when the display is rotated (as registered in the manifest). */
    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);

        mDrawerToggle.onConfigurationChanged(newConfig);
    }

    /** Called to initialize the action bar. */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.chromoting_actionbar, menu);
        mRefreshButton = menu.findItem(R.id.actionbar_directoryrefresh);

        if (mAccount == null) {
            // If there is no account, don't allow the user to refresh the listing.
            mRefreshButton.setEnabled(false);
        }

        return super.onCreateOptionsMenu(menu);
    }

    /** Called whenever an action bar button is pressed. */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (mDrawerToggle.onOptionsItemSelected(item)) {
            return true;
        }

        int id = item.getItemId();
        if (id == R.id.actionbar_directoryrefresh) {
            refreshHostList();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    /** Called when the user touches hyperlinked text. */
    @Override
    public void onClick(View view) {
        HelpActivity.launch(this, HOST_SETUP_URL);
    }

    /** Called when the user taps on a host entry. */
    public void connectToHost(HostInfo host) {
        mProgressIndicator = ProgressDialog.show(
                this,
                host.name,
                getString(R.string.footer_connecting),
                true,
                true,
                new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        JniInterface.disconnectFromHost();
                        mTokenFetcher = null;
                    }
                });
        SessionConnector connector = new SessionConnector(this, this, mHostListLoader);
        assert mTokenFetcher == null;
        mTokenFetcher = createTokenFetcher(host);
        connector.connectToHost(mAccount.name, mToken, host);
    }

    private void refreshHostList() {
        if (mWaitingForAuthToken) {
            return;
        }

        mTriedNewAuthToken = false;
        setHostListProgressVisible(true);

        // The refresh button simply makes use of the currently-chosen account.
        requestAuthToken();
    }

    private void requestAuthToken() {
        AccountManager.get(this).getAuthToken(mAccount, TOKEN_SCOPE, null, this, this, null);
        mWaitingForAuthToken = true;
    }

    @Override
    public void run(AccountManagerFuture<Bundle> future) {
        Log.i("auth", "User finished with auth dialogs");
        mWaitingForAuthToken = false;

        Bundle result = null;
        String explanation = null;
        try {
            // Here comes our auth token from the Android system.
            result = future.getResult();
        } catch (OperationCanceledException ex) {
            // User canceled authentication. No need to report an error.
        } catch (AuthenticatorException ex) {
            explanation = getString(R.string.error_unexpected);
        } catch (IOException ex) {
            explanation = getString(R.string.error_network_error);
        }

        if (result == null) {
            setHostListProgressVisible(false);
            if (explanation != null) {
                Toast.makeText(this, explanation, Toast.LENGTH_LONG).show();
            }
            return;
        }

        mToken = result.getString(AccountManager.KEY_AUTHTOKEN);
        Log.i("auth", "Received an auth token from system");

        mHostListLoader.retrieveHostList(mToken, this);
    }

    @Override
    public void onItemSelected(AdapterView parent, View view, int itemPosition, long itemId) {
        mAccount = mAccounts[itemPosition];

        getPreferences(MODE_PRIVATE).edit().putString("account_name", mAccount.name)
                .putString("account_type", mAccount.type).apply();

        // The current host list is no longer valid for the new account, so clear the list.
        mHosts = new HostInfo[0];
        updateUi();
        refreshHostList();
    }

    @Override
    public void onNothingSelected(AdapterView parent) {
    }

    @Override
    public void onHostListReceived(HostInfo[] hosts) {
        // Store a copy of the array, so that it can't be mutated by the HostListLoader. HostInfo
        // is an immutable type, so a shallow copy of the array is sufficient here.
        mHosts = Arrays.copyOf(hosts, hosts.length);
        setHostListProgressVisible(false);
        updateUi();
    }

    @Override
    public void onError(HostListLoader.Error error) {
        String explanation = null;
        switch (error) {
            case AUTH_FAILED:
                break;
            case NETWORK_ERROR:
                explanation = getString(R.string.error_network_error);
                break;
            case UNEXPECTED_RESPONSE:
            case SERVICE_UNAVAILABLE:
            case UNKNOWN:
                explanation = getString(R.string.error_unexpected);
                break;
            default:
                // Unreachable.
                return;
        }

        if (explanation != null) {
            Toast.makeText(this, explanation, Toast.LENGTH_LONG).show();
            setHostListProgressVisible(false);
            return;
        }

        // This is the AUTH_FAILED case.

        if (!mTriedNewAuthToken) {
            // This was our first connection attempt.

            AccountManager authenticator = AccountManager.get(this);
            mTriedNewAuthToken = true;

            Log.w("auth", "Requesting renewal of rejected auth token");
            authenticator.invalidateAuthToken(mAccount.type, mToken);
            mToken = null;
            requestAuthToken();

            // We're not in an error state *yet*.
            return;
        } else {
            // Authentication truly failed.
            Log.e("auth", "Fresh auth token was also rejected");
            explanation = getString(R.string.error_authentication_failed);
            Toast.makeText(this, explanation, Toast.LENGTH_LONG).show();
            setHostListProgressVisible(false);
        }
    }

    /**
     * Updates the infotext and host list display.
     */
    private void updateUi() {
        if (mRefreshButton != null) {
            mRefreshButton.setEnabled(mAccount != null);
        }
        ArrayAdapter<HostInfo> displayer = new HostListAdapter(this, R.layout.host, mHosts);
        Log.i("hostlist", "About to populate host list display");
        mHostListView.setAdapter(displayer);
    }

    @Override
    public void onConnectionState(JniInterface.ConnectionListener.State state,
            JniInterface.ConnectionListener.Error error) {
        boolean dismissProgress = false;
        switch (state) {
            case INITIALIZING:
            case CONNECTING:
            case AUTHENTICATED:
                // The connection is still being established.
                break;

            case CONNECTED:
                dismissProgress = true;
                // Display the remote desktop.
                startActivityForResult(new Intent(this, Desktop.class), 0);
                break;

            case FAILED:
                dismissProgress = true;
                Toast.makeText(this, getString(error.message()), Toast.LENGTH_LONG).show();
                // Close the Desktop view, if it is currently running.
                finishActivity(0);
                break;

            case CLOSED:
                // No need to show toast in this case. Either the connection will have failed
                // because of an error, which will trigger toast already. Or the disconnection will
                // have been initiated by the user.
                dismissProgress = true;
                finishActivity(0);
                break;

            default:
                // Unreachable, but required by Google Java style and findbugs.
                assert false : "Unreached";
        }

        if (dismissProgress && mProgressIndicator != null) {
            mProgressIndicator.dismiss();
            mProgressIndicator = null;
        }
    }

    private ThirdPartyTokenFetcher createTokenFetcher(HostInfo host) {
        ThirdPartyTokenFetcher.Callback callback = new ThirdPartyTokenFetcher.Callback() {
            @Override
            public void onTokenFetched(String code, String accessToken) {
                // The native client sends the OAuth authorization code to the host as the token so
                // that the host can obtain the shared secret from the third party authorization
                // server.
                String token = code;

                // The native client uses the OAuth access token as the shared secret to
                // authenticate itself with the host using spake.
                String sharedSecret = accessToken;

                JniInterface.onThirdPartyTokenFetched(token, sharedSecret);
            }
        };
        return new ThirdPartyTokenFetcher(this, host.getTokenUrlPatterns(), callback);
    }

    public void fetchThirdPartyToken(String tokenUrl, String clientId, String scope) {
        assert mTokenFetcher != null;
        mTokenFetcher.fetchToken(tokenUrl, clientId, scope);
    }
}
