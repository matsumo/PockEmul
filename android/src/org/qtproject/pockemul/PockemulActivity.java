/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.qtproject.pockemul;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.ComponentName;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.Intent.ShortcutIconResource;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.os.Looper;
import android.os.Vibrator;
import android.util.Log;
import android.provider.Settings;
import android.provider.Settings.System;
import android.util.Log;
import android.webkit.MimeTypeMap;
import android.view.WindowManager;

import java.io.File;
import java.util.concurrent.Semaphore;

import org.qtproject.qt5.android.bindings.QtActivity;


public class PockemulActivity extends QtActivity {

    private static PockemulActivity m_instance;
    public PockemulActivity()
    {
        m_instance = this;
    }


    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        //get the received intent
        Intent receivedIntent = getIntent();
        //get the action
        String receivedAction = receivedIntent.getAction();
        args = receivedIntent.getStringExtra("args");
        Log.i("Qt", "*******************************");
        if (receivedAction != null) {
            Log.i("Qt", receivedAction);

            // and get whatever type user account id is
        }
        if (args != null) {
            Log.i("Qt", args);


            // and get whatever type user account id is
        }
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
    }



    /** If no pattern was found, vibrate for a small amount of time. */
    private static final long DURATION = 10;  // millisec.
    /** Play the haptic pattern only once. */
    private static final int NO_REPEAT = -1;

    private static final String TAG = "Pockemul";
    private Context mContext;
    private long[] mHapticPattern;
    private Vibrator mVibrator;

    private boolean mEnabled;
    private Settings.System mSystemSettings;
    private ContentResolver mContentResolver;
    private boolean mSettingEnabled;

    private static String args;
    public static String getArgs()
    {
        return args;
    }

    private static final Semaphore dialogSemaphore = new Semaphore(0, true);


    private static int pressedButtonID;
    public static int ShowMyModalDialog(String msg,int nb)  //should be called from non-UI thread
    {

        pressedButtonID=-1;
        final String dialogMsg = msg;
        final int nbButtons = nb;
        Log.i("Qt", "Enter ShowMyModalDialog");
        m_instance.runOnUiThread( new Runnable()
        {
            public void run()
            {
            String butlbl = "Yes";
            if (nbButtons==1) butlbl="Ok";
                AlertDialog errorDialog = new AlertDialog.Builder( m_instance ).create();
                errorDialog.setMessage(dialogMsg);
                errorDialog.setButton(butlbl, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        pressedButtonID = 1;
                        dialogSemaphore.release();
                        }
                    });
                 if (nbButtons>1) {
                        errorDialog.setButton2("No", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                pressedButtonID = 2;
                                dialogSemaphore.release();
                                }
                            });
                            }
                 if (nbButtons==3) {
                    errorDialog.setButton3("Cancel", new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            pressedButtonID = 3;
                            dialogSemaphore.release();
                            }
                        });
                    }
                errorDialog.setCancelable(false);

               // errorDialog.getWindow().setFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
               //         WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);

                        //Set the dialog to immersive
                //errorDialog.getWindow().getDecorView().setSystemUiVisibility(
                //        m_instance.getWindow().getDecorView().getSystemUiVisibility());
                errorDialog.show();

                // Set the dialog to focusable again.
                //errorDialog.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);
            };
        }
        );
        try
        {
            dialogSemaphore.acquire();
        }
        catch (InterruptedException e)
        {
        }
//        Vibrate();
        return pressedButtonID;
    }

    public static void Vibrate(int nb)
    {
        // Get instance of the Vibrator
         Vibrator vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
         // Vibrate for nb milliseconds
         vibrator.vibrate(nb);
    }

    public static void openURL(String url)
    {

        Intent intent = new Intent(Intent.ACTION_VIEW);
        MimeTypeMap mime = MimeTypeMap.getSingleton();
        File file = new File(url);
        String ext=file.getName().substring(file.getName().lastIndexOf(".")+1).toLowerCase();
        String type = mime.getMimeTypeFromExtension(ext);
        intent.setDataAndType(Uri.fromFile(file),type);
        m_instance.startActivity(intent);

    }

    public static void addShortcut(String name,String param) {
        //Adding shortcut for MainActivity
        //on Home screen
        Intent shortcut = new Intent("com.android.launcher.action.INSTALL_SHORTCUT");

        // Shortcut name
        shortcut.putExtra(Intent.EXTRA_SHORTCUT_NAME, name);
        shortcut.putExtra("duplicate", false);  // Just create once

        // Setup current activity shoud be shortcut object
        ComponentName comp = new ComponentName("org.qtproject.pockemul","org.qtproject.pockemul.PockemulActivity");
        Intent intent = new Intent(Intent.ACTION_MAIN).setComponent(comp);
        intent.putExtra("args",param);
        shortcut.putExtra(Intent.EXTRA_SHORTCUT_INTENT, intent);

        // Set shortcut icon
        ShortcutIconResource iconRes = Intent.ShortcutIconResource.fromContext(m_instance, R.drawable.icon);
        shortcut.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE, iconRes);

        m_instance.sendBroadcast(shortcut);
    }

}

