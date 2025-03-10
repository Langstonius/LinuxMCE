/*
    Copyright (c) 2009-2011, BogDan Vatra <bog_dan_ro@yahoo.com>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the  BogDan Vatra <bog_dan_ro@yahoo.com> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY BogDan Vatra <bog_dan_ro@yahoo.com> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL BogDan Vatra <bog_dan_ro@yahoo.com> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

package org.kde.necessitas.origo;


import java.io.File;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;

import org.kde.necessitas.ministro.IMinistro;
import org.kde.necessitas.ministro.IMinistroCallback;
import org.kde.necessitas.origo.LinuxmceAudioService;
import org.kde.necessitas.origo.LinuxmceAudioService.LocalBinder;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.content.res.Resources.Theme;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.net.Uri;
import android.os.Binder;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.AttributeSet;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.view.accessibility.AccessibilityEvent;
import dalvik.system.DexClassLoader;


//@ANDROID-11
//QtCreator import android.app.Fragment;
//QtCreator import android.view.ActionMode;
//QtCreator import android.view.ActionMode.Callback;
//@ANDROID-11

public class QtActivity<LocalBinder> extends Activity
{
    private final static int MINISTRO_INSTALL_REQUEST_CODE = 0xf3ee; // request code used to know when Ministro instalation is finished
    private static final int MINISTRO_API_LEVEL=2; // Ministro api level (check IMinistro.aidl file)
    private static final int NECESSITAS_API_LEVEL=2; // Necessitas api level used by platform plugin
    private static final String QT_PROVIDER="necessitas";
    private static final int QT_VERSION=0x040801; // Qt version 4.8.00 check http://qt-project.org/doc/qt-4.8/qtglobal.html#QT_VERSION

    private static final String ERROR_CODE_KEY="error.code";
    private static final String ERROR_MESSAGE_KEY="error.message";
    private static final String DEX_PATH_KEY="dex.path";
    private static final String LIB_PATH_KEY="lib.path";
    private static final String LOADER_CLASS_NAME_KEY="loader.class.name";
    private static final String NATIVE_LIBRARIES_KEY="native.libraries";
    private static final String ENVIRONMENT_VARIABLES_KEY="environment.variables";
    private static final String APPLICATION_PARAMETERS_KEY="application.parameters";
    private static final String BUNDLED_LIBRARIES_KEY="bundled.libraries";
    private static final String MAIN_LIBRARY_KEY="main.library";
    private static final String NECESSITAS_API_LEVEL_KEY="necessitas.api.level";

    /// Ministro server parameter keys
    private static final String REQUIRED_MODULES_KEY="required.modules";
    private static final String APPLICATION_TITLE_KEY="application.title";
    private static final String QT_PROVIDER_KEY="qt.provider";
    private static final String MINIMUM_MINISTRO_API_KEY="minimum.ministro.api";
    private static final String MINIMUM_QT_VERSION_KEY="minimum.qt.version";
//    private static final String REPOSITORIES="3rd.party.repositories"; // needs MINISTRO_API_LEVEL >=2 !!!
                                                                       // Use this key to specify any 3rd party repositories urls
                                                                       // Ministro will download these repositories into thier
                                                                       // own folders, check http://community.kde.org/Necessitas/Ministro
                                                                       // for more details.

    private static final String APPLICATION_PARAMETERS=null; // use this variable to pass any parameters to your application,
                                                             // the parameters must not contain any white spaces
                                                             // and must be separated with "\t"
                                                             // e.g "-param1\t-param2=value2\t-param3\tvalue3"

    private static final String ENVIRONMENT_VARIABLES="QT_USE_ANDROID_NATIVE_STYLE=1\t";
                                                             // use this variable to add any environment variables to your application.
                                                             // the env vars must be separated with "\t"
                                                             // e.g. "ENV_VAR1=1\tENV_VAR2=2\t"
                                                             // Currently the following vars are used by the android plugin:
                                                             // * QT_USE_ANDROID_NATIVE_STYLE - 0 if you don't want to use android style plugin, it will save a few ms at startup.

    private static final int INCOMPATIBLE_MINISTRO_VERSION=1; // Incompatible Ministro version. Ministro needs to be upgraded.
    private ActivityInfo m_activityInfo = null; // activity info object, used to access the libs and the strings
    private DexClassLoader m_classLoader = null; // loader object
    private String[] m_qtLibs = null; // required qt libs


    public static QtActivity qtactivity;

    LinuxmceAudioService mService;
    public long myobj;

    boolean mBound = false;

    public static QtActivity getActivity() {
      return qtactivity;
    }

    public void PlayMedia(String url) {    
    		  mService.playAudio(url);   
    }

    public boolean SendMediaCommand(String Command, int mSeek, boolean pause, String file, float vol){
        boolean res = false;
        if(mService==null){
    		return false;
    	}
        if(Command.contentEquals("play")){
                mService.playAudio(file);
                res = true;
        }
        else if(Command.contentEquals("stop")){
        mService.stop();
        res = true;
        }
        else if(Command.contentEquals("setVolume")){
                mService.setVolume(vol);
                res=true;
        }

        return res;
    }
    
    public int currentMediaPosition(){
    	if(mService==null){
    		return 0;
    	}
    	return mService.getElapsed();
    }
    
    public boolean seekToPosition(int msec){ 
    	if(mService==null){
    		return false;
    	}
    	return mService.seek(msec);   	    
    }
    
    public boolean setVolumeLevel(float vol){
    	mService.setVolume(vol);
    	return true;
    }
    
    public boolean stopMedia(){
    	if(mService==null){
    		return false;
    	}
    	return mService.stop();
    }
    
    public boolean pauseMedia(){    
    	if(mService==null){
    		return false;
    	}
    	return mService.pause();
    }
    
    public boolean resumeMedia(){
    	if(mService==null){
    		return false;
    	}
    	return mService.resume();
    }
    
    public int getElapsedTime(){
    	if(mService==null){
    		return 0;
    	}
    	return mService.getElapsed();
    }


    public void startAudioService(long t){
        Log.d("Linuxmce Audio Service", "Trying to start Linuxmce Audio Service");
                Intent i = new Intent(this , LinuxmceAudioService.class);
                if(bindService(i, mConnection, this.BIND_AUTO_CREATE)){

                }

    }

    public void playAudioFile(String file){
        mService.playAudio(file);
    }

    /** Defines callbacks for service binding, passed to bindService() */
    private ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className,
                IBinder service) {
            // We've bound to LocalService, cast the IBinder and get LocalService instance
            LocalBinder binder = (LocalBinder) service;
            mService = ((LinuxmceAudioService.LocalBinder)service).getService();
            mBound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mBound = false;
        }
    };




  //This version sends an implicit intent that android will handle for us.
//    public void playMedia(String url) {
//        Uri uri = Uri.parse(url);

//        startActivity(new Intent(Intent.ACTION_VIEW, uri));
//    }

    // this function is used to load and start the loader
    private void loadApplication(Bundle loaderParams)
    {

        try
        {
            final int errorCode = loaderParams.getInt(ERROR_CODE_KEY);
            if (errorCode != 0)
            {
                if (errorCode == INCOMPATIBLE_MINISTRO_VERSION)
                {
                    downloadUpgradeMinistro(loaderParams.getString(ERROR_MESSAGE_KEY));
                    return;
                }

                // fatal error, show the error and quit
                AlertDialog errorDialog = new AlertDialog.Builder(QtActivity.this).create();
                errorDialog.setMessage(loaderParams.getString(ERROR_MESSAGE_KEY));
                errorDialog.setButton(getResources().getString(android.R.string.ok), new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
                    }
                });
                errorDialog.show();
                return;
            }

            // add all bundled Qt libs to loader params
            ArrayList<String> libs = new ArrayList<String>();
            if ( m_activityInfo.metaData.containsKey("android.app.bundled_libs_resource_id") )
                libs.addAll(Arrays.asList(getResources().getStringArray(m_activityInfo.metaData.getInt("android.app.bundled_libs_resource_id"))));

            String libName = null;
            if ( m_activityInfo.metaData.containsKey("android.app.lib_name") ) {
                libName = m_activityInfo.metaData.getString("android.app.lib_name");
                loaderParams.putString(MAIN_LIBRARY_KEY, libName); //main library contains main() function
            }

            loaderParams.putStringArrayList(BUNDLED_LIBRARIES_KEY, libs);
            loaderParams.putInt(NECESSITAS_API_LEVEL_KEY, NECESSITAS_API_LEVEL);

            // load and start QtLoader class
            m_classLoader = new DexClassLoader(loaderParams.getString(DEX_PATH_KEY) // .jar/.apk files
                                            , getDir("outdex", Context.MODE_PRIVATE).getAbsolutePath() // directory where optimized DEX files should be written.
                                            , loaderParams.containsKey(LIB_PATH_KEY)?loaderParams.getString(LIB_PATH_KEY):null // libs folder (if exists)
                                            , getClassLoader()); // parent loader

            @SuppressWarnings("rawtypes")
            Class loaderClass = m_classLoader.loadClass(loaderParams.getString(LOADER_CLASS_NAME_KEY)); // load QtLoader class
            Object qtLoader = loaderClass.newInstance(); // create an instance
            Method perpareAppMethod=qtLoader.getClass().getMethod("loadApplication", Activity.class, ClassLoader.class, Bundle.class);
            if (!(Boolean)perpareAppMethod.invoke(qtLoader, this, m_classLoader, loaderParams))
                throw new Exception("");

            QtApplication.setQtActivityDelegate(qtLoader);

            // now load the application library so it's accessible from this class loader
            if (libName != null)
                System.loadLibrary(libName);

            Method startAppMethod=qtLoader.getClass().getMethod("startApplication");
            if (!(Boolean)startAppMethod.invoke(qtLoader))
                throw new Exception("");

        } catch (Exception e) {
            e.printStackTrace();
            AlertDialog errorDialog = new AlertDialog.Builder(QtActivity.this).create();
            if (m_activityInfo != null && m_activityInfo.metaData.containsKey("android.app.fatal_error_msg"))
                errorDialog.setMessage(m_activityInfo.metaData.getString("android.app.fatal_error_msg"));
            else
                errorDialog.setMessage("Fatal error, your application can't be started.");
            errorDialog.setButton(getResources().getString(android.R.string.ok), new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            });
            errorDialog.show();
        }

    }

    private ServiceConnection m_ministroConnection=new ServiceConnection() {
        private IMinistro m_service = null;
    @Override
        public void onServiceConnected(ComponentName name, IBinder service)
        {
            m_service = IMinistro.Stub.asInterface(service);
            try {
                if (m_service!=null)
                {
                    Bundle parameters= new Bundle();
                    parameters.putStringArray(REQUIRED_MODULES_KEY, m_qtLibs);
                    parameters.putString(APPLICATION_TITLE_KEY, (String)QtActivity.this.getTitle());
                    parameters.putInt(MINIMUM_MINISTRO_API_KEY, MINISTRO_API_LEVEL);
                    parameters.putString(QT_PROVIDER_KEY, QT_PROVIDER);
                    parameters.putInt(MINIMUM_QT_VERSION_KEY, QT_VERSION);
                    parameters.putString(ENVIRONMENT_VARIABLES_KEY, ENVIRONMENT_VARIABLES);
                    if (null!=APPLICATION_PARAMETERS)
                        parameters.putString(APPLICATION_PARAMETERS_KEY, APPLICATION_PARAMETERS);
                    // parameters.putStringArray(REPOSITORIES, null);
                    m_service.requestLoader(m_ministroCallback, parameters);
                }
            } catch (RemoteException e) {
                    e.printStackTrace();
            }
        }

    private IMinistroCallback m_ministroCallback = new IMinistroCallback.Stub() {
        // this function is called back by Ministro.
        @Override
        public void loaderReady(final Bundle loaderParams) throws RemoteException
        {
            runOnUiThread( new Runnable() {
                @Override
                public void run() {
                    unbindService(m_ministroConnection);
                    loadApplication(loaderParams);
                }
            });
        }
    };

        @Override
        public void onServiceDisconnected(ComponentName name)
        {
            m_service = null;
        }
    };

    private void downloadUpgradeMinistro(String msg)
    {
        AlertDialog.Builder downloadDialog = new AlertDialog.Builder(this);
        downloadDialog.setMessage(msg);
        downloadDialog.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                try
                {
                    Uri uri = Uri.parse("market://search?q=pname:org.kde.necessitas.ministro");
                    Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                    startActivityForResult(intent, MINISTRO_INSTALL_REQUEST_CODE);
                }
                catch (Exception e) {
                    e.printStackTrace();
                    ministroNotFound();
                }
            }
        });

        downloadDialog.setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                QtActivity.this.finish();
            }
        });
        downloadDialog.show();
    }

    private void ministroNotFound()
    {
        AlertDialog errorDialog = new AlertDialog.Builder(QtActivity.this).create();

        if (m_activityInfo != null && m_activityInfo.metaData.containsKey("android.app.ministro_not_found_msg"))
            errorDialog.setMessage(m_activityInfo.metaData.getString("android.app.ministro_not_found_msg"));
        else
            errorDialog.setMessage("Can't find Ministro service.\nThe application can't start.");

        errorDialog.setButton(getResources().getString(android.R.string.ok), new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        });
        errorDialog.show();
    }

    private void startApp(final boolean firstStart)
    {
        try
        {
            ActivityInfo ai=getPackageManager().getActivityInfo(getComponentName(), PackageManager.GET_META_DATA);
            if (ai.metaData.containsKey("android.app.qt_libs_resource_id"))
            {
                int resourceId = ai.metaData.getInt("android.app.qt_libs_resource_id");
                m_qtLibs=getResources().getStringArray(resourceId);
            }
            if (getIntent().getExtras()!= null && getIntent().getExtras().containsKey("use_local_qt_libs")
                    && getIntent().getExtras().getString("use_local_qt_libs").equals("true"))
            {
                ArrayList<String> libraryList= new ArrayList<String>();

                String localPrefix="/data/local/qt/";
                if (getIntent().getExtras().containsKey("libs_prefix"))
                    localPrefix=getIntent().getExtras().getString("libs_prefix");

                if (m_qtLibs != null)
                    for(int i=0;i<m_qtLibs.length;i++)
                    {
                        libraryList.add(localPrefix+"lib/lib"+m_qtLibs[i]+".so");
                    }

                if (getIntent().getExtras().containsKey("load_local_libs"))
                {
                    String []extraLibs=getIntent().getExtras().getString("load_local_libs").split(":");
                    for (String lib:extraLibs)
                        if (lib.length()>0)
                            libraryList.add(localPrefix+lib);
                }

                String dexPaths = new String();
                String pathSeparator = System.getProperty("path.separator", ":");
                if (getIntent().getExtras().containsKey("load_local_jars"))
                {
                    String []jarFiles=getIntent().getExtras().getString("load_local_jars").split(":");
                    for (String jar:jarFiles)
                        if (jar.length()>0)
                        {
                            if (dexPaths.length()>0)
                                dexPaths+=pathSeparator;
                            dexPaths+=localPrefix+jar;
                        }
                }

                Bundle loaderParams = new Bundle();
                loaderParams.putInt(ERROR_CODE_KEY, 0);
                loaderParams.putString(DEX_PATH_KEY, dexPaths);
                loaderParams.putString(LOADER_CLASS_NAME_KEY, getIntent().getExtras().containsKey("loader_class_name")
                                                            ?getIntent().getExtras().getString("loader_class_name")
                                                            :"org.kde.necessitas.industrius.QtActivityDelegate");
                loaderParams.putStringArrayList(NATIVE_LIBRARIES_KEY, libraryList);
                loaderParams.putString(ENVIRONMENT_VARIABLES_KEY,"QML_IMPORT_PATH="+localPrefix+"/imports\tQT_PLUGIN_PATH="+localPrefix+"/plugins");
                loaderParams.putString(APPLICATION_PARAMETERS_KEY,"-platform\tandroid");
                loadApplication(loaderParams);
                return;
            }

            try {
                if (!bindService(new Intent(org.kde.necessitas.ministro.IMinistro.class.getCanonicalName()), m_ministroConnection, Context.BIND_AUTO_CREATE))
                    throw new SecurityException("");
            } catch (Exception e) {
                if (firstStart)
                {
                    String msg="This application requires Ministro service. Would you like to install it?";
                    if (m_activityInfo != null && m_activityInfo.metaData.containsKey("android.app.ministro_needed_msg"))
                        msg=m_activityInfo.metaData.getString("android.app.ministro_needed_msg");
                    downloadUpgradeMinistro(msg);
                }
                else
                {
                    ministroNotFound();
                }
            }
        }
        catch (Exception e)
        {
            Log.e(QtApplication.QtTAG, "Can't create main activity", e);
        }
    }



    /////////////////////////// forward all notifications ////////////////////////////
    /////////////////////////// Super class calls ////////////////////////////////////
    /////////////// PLEASE DO NOT CHANGE THE FOLLOWING CODE //////////////////////////
    //////////////////////////////////////////////////////////////////////////////////

    @Override
    public boolean dispatchKeyEvent(KeyEvent event)
    {
        if (QtApplication.m_delegateObject != null && QtApplication.dispatchKeyEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchKeyEvent, event);
        else
            return super.dispatchKeyEvent(event);
    }
    public boolean super_dispatchKeyEvent(KeyEvent event)
    {
        return super.dispatchKeyEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean dispatchPopulateAccessibilityEvent(AccessibilityEvent event)
    {
        if (QtApplication.m_delegateObject != null && QtApplication.dispatchPopulateAccessibilityEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchPopulateAccessibilityEvent, event);
        else
            return super.dispatchPopulateAccessibilityEvent(event);
    }
    public boolean super_dispatchPopulateAccessibilityEvent(AccessibilityEvent event)
    {
        return super_dispatchPopulateAccessibilityEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev)
    {
        if (QtApplication.m_delegateObject != null && QtApplication.dispatchTouchEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchTouchEvent, ev);
        else
            return super.dispatchTouchEvent(ev);
    }
    public boolean super_dispatchTouchEvent(MotionEvent event)
    {
        return super.dispatchTouchEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean dispatchTrackballEvent(MotionEvent ev)
    {
        if (QtApplication.m_delegateObject != null && QtApplication.dispatchTrackballEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchTrackballEvent, ev);
        else
            return super.dispatchTrackballEvent(ev);
    }
    public boolean super_dispatchTrackballEvent(MotionEvent event)
    {
        return super.dispatchTrackballEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {

        if (QtApplication.m_delegateObject != null && QtApplication.onActivityResult != null)
        {
            QtApplication.invokeDelegateMethod(QtApplication.onActivityResult, requestCode, resultCode, data);
            return;
        }
        if (requestCode == MINISTRO_INSTALL_REQUEST_CODE)
                startApp(false);
        super.onActivityResult(requestCode, resultCode, data);
    }
    public void super_onActivityResult(int requestCode, int resultCode, Intent data)
    {
        super.onActivityResult(requestCode, resultCode, data);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onApplyThemeResource(Theme theme, int resid, boolean first)
    {
        if (!QtApplication.invokeDelegate(theme, resid, first).invoked)
            super.onApplyThemeResource(theme, resid, first);
    }
    public void super_onApplyThemeResource(Theme theme, int resid, boolean first)
    {
        super.onApplyThemeResource(theme, resid, first);
    }
    //---------------------------------------------------------------------------


    @Override
    protected void onChildTitleChanged(Activity childActivity, CharSequence title)
    {
        if (!QtApplication.invokeDelegate(childActivity, title).invoked)
            super.onChildTitleChanged(childActivity, title);
    }
    public void super_onChildTitleChanged(Activity childActivity, CharSequence title)
    {
        super.onChildTitleChanged(childActivity, title);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onConfigurationChanged(Configuration newConfig)
    {
        if (!QtApplication.invokeDelegate(newConfig).invoked)
            super.onConfigurationChanged(newConfig);
    }
    public void super_onConfigurationChanged(Configuration newConfig)
    {
        super.onConfigurationChanged(newConfig);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onContentChanged()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onContentChanged();
    }
    public void super_onContentChanged()
    {
        super.onContentChanged();
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onContextItemSelected(MenuItem item)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(item);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onContextItemSelected(item);
    }
    public boolean super_onContextItemSelected(MenuItem item)
    {
        return super.onContextItemSelected(item);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onContextMenuClosed(Menu menu)
    {
        if (!QtApplication.invokeDelegate(menu).invoked)
            super.onContextMenuClosed(menu);
    }
    public void super_onContextMenuClosed(Menu menu)
    {
        super.onContextMenuClosed(menu);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        qtactivity = this;
        super.onCreate(savedInstanceState);
        if (QtApplication.m_delegateObject != null && QtApplication.onCreate != null)
        {
            QtApplication.invokeDelegateMethod(QtApplication.onCreate, savedInstanceState);
            return;
        }
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        try {
            m_activityInfo = getPackageManager().getActivityInfo(getComponentName(), PackageManager.GET_META_DATA);
        } catch (NameNotFoundException e) {
            e.printStackTrace();
            finish();
            return;
        }
        if (null == getLastNonConfigurationInstance())
        {
            // if splash screen is defined, then show it
            if ( m_activityInfo.metaData.containsKey("android.app.splash_screen") )
                setContentView(m_activityInfo.metaData.getInt("android.app.splash_screen"));
            startApp(true);
        }

    }
    //---------------------------------------------------------------------------

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo)
    {
        if (!QtApplication.invokeDelegate(menu, v, menuInfo).invoked)
            super.onCreateContextMenu(menu, v, menuInfo);
    }
    public void super_onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo)
    {
        super.onCreateContextMenu(menu, v, menuInfo);
    }
    //---------------------------------------------------------------------------

    @Override
    public CharSequence onCreateDescription()
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate();
        if (res.invoked)
            return (CharSequence)res.methodReturns;
        else
            return super.onCreateDescription();
    }
    public CharSequence super_onCreateDescription()
    {
        return super.onCreateDescription();
    }
    //---------------------------------------------------------------------------

    @Override
    protected Dialog onCreateDialog(int id)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(id);
        if (res.invoked)
            return (Dialog)res.methodReturns;
        else
            return super.onCreateDialog(id);
    }
    public Dialog super_onCreateDialog(int id)
    {
        return super.onCreateDialog(id);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(menu);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onCreateOptionsMenu(menu);
    }
    public boolean super_onCreateOptionsMenu(Menu menu)
    {
        return super.onCreateOptionsMenu(menu);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onCreatePanelMenu(int featureId, Menu menu)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId, menu);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onCreatePanelMenu(featureId, menu);
    }
    public boolean super_onCreatePanelMenu(int featureId, Menu menu)
    {
        return super.onCreatePanelMenu(featureId, menu);
    }
    //---------------------------------------------------------------------------


    @Override
    public View onCreatePanelView(int featureId)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId);
        if (res.invoked)
            return (View)res.methodReturns;
        else
            return super.onCreatePanelView(featureId);
    }
    public View super_onCreatePanelView(int featureId)
    {
        return super.onCreatePanelView(featureId);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onCreateThumbnail(Bitmap outBitmap, Canvas canvas)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(outBitmap, canvas);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onCreateThumbnail(outBitmap, canvas);
    }
    public boolean super_onCreateThumbnail(Bitmap outBitmap, Canvas canvas)
    {
        return super.onCreateThumbnail(outBitmap, canvas);
    }
    //---------------------------------------------------------------------------

    @Override
    public View onCreateView(String name, Context context, AttributeSet attrs)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(name, context, attrs);
        if (res.invoked)
            return (View)res.methodReturns;
        else
            return super.onCreateView(name, context, attrs);
    }
    public View super_onCreateView(String name, Context context, AttributeSet attrs)
    {
        return super.onCreateView(name, context, attrs);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------


@Override
    public boolean onKeyDown(int keyCode, KeyEvent event)
    {
        int newKeyCode = keyCode;
        if ( (keyCode == KeyEvent.KEYCODE_BACK) )
        {
           newKeyCode = KeyEvent.KEYCODE_MEDIA_PREVIOUS;
        }
        if (QtApplication.m_delegateObject != null && QtApplication.onKeyDown != null)
            return (Boolean)
            QtApplication.invokeDelegateMethod(QtApplication.onKeyDown, newKeyCode,event);
        else
            return super.onKeyDown(newKeyCode, event);
    }
    public boolean super_onKeyDown(int keyCode, KeyEvent event)
    {
        return super.onKeyDown(keyCode, event);
    }

//---------------------------------------------------------------------------

 @Override
    public boolean onKeyMultiple(int keyCode, int repeatCount, KeyEvent event)
    {
        int newKeyCode = keyCode;
        if ( (keyCode == KeyEvent.KEYCODE_BACK) )
        {
            newKeyCode = KeyEvent.KEYCODE_MEDIA_PREVIOUS;
        }
        if (QtApplication.m_delegateObject != null && QtApplication.onKeyMultiple != null)
            return (Boolean)
            QtApplication.invokeDelegateMethod(QtApplication.onKeyMultiple ,newKeyCode, repeatCount, event);
        else
            return super.onKeyMultiple(newKeyCode, repeatCount, event);
    }

    public boolean super_onKeyMultiple(int keyCode, int repeatCount,KeyEvent event)
    {
        return super.onKeyMultiple(keyCode, repeatCount, event);
    }

//---------------------------------------------------------------------------

@Override
    public boolean onKeyUp(int keyCode, KeyEvent event)
    {
        int newKeyCode = keyCode;
        if ( (keyCode == KeyEvent.KEYCODE_BACK) )
        {
            newKeyCode = KeyEvent.KEYCODE_MEDIA_PREVIOUS;
        }
        if (QtApplication.m_delegateObject != null  && QtApplication.onKeyDown != null)
            return (Boolean)
QtApplication.invokeDelegateMethod(QtApplication.onKeyUp, newKeyCode, event);
        else
            return super.onKeyUp(newKeyCode, event);
    }
    public boolean super_onKeyUp(int keyCode, KeyEvent event)
    {
        return super.onKeyUp(keyCode, event);
    }
//---------------------------------------------------------------------------

    @Override
    public void onLowMemory()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onLowMemory();
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId, item);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onMenuItemSelected(featureId, item);
    }
    public boolean super_onMenuItemSelected(int featureId, MenuItem item)
    {
        return super.onMenuItemSelected(featureId, item);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onMenuOpened(int featureId, Menu menu)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId, menu);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onMenuOpened(featureId, menu);
    }
    public boolean super_onMenuOpened(int featureId, Menu menu)
    {
        return super.onMenuOpened(featureId, menu);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onNewIntent(Intent intent)
    {
        if (!QtApplication.invokeDelegate(intent).invoked)
            super.onNewIntent(intent);
    }
    public void super_onNewIntent(Intent intent)
    {
        super.onNewIntent(intent);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(item);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onOptionsItemSelected(item);
    }
    public boolean super_onOptionsItemSelected(MenuItem item)
    {
        return super.onOptionsItemSelected(item);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onOptionsMenuClosed(Menu menu)
    {
        if (!QtApplication.invokeDelegate(menu).invoked)
            super.onOptionsMenuClosed(menu);
    }
    public void super_onOptionsMenuClosed(Menu menu)
    {
        super.onOptionsMenuClosed(menu);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onPanelClosed(int featureId, Menu menu)
    {
        if (!QtApplication.invokeDelegate(featureId, menu).invoked)
            super.onPanelClosed(featureId, menu);
    }
    public void super_onPanelClosed(int featureId, Menu menu)
    {
        super.onPanelClosed(featureId, menu);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPause()
    {
        super.onPause();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPostCreate(Bundle savedInstanceState)
    {
        super.onPostCreate(savedInstanceState);
        QtApplication.invokeDelegate(savedInstanceState);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPostResume()
    {
        super.onPostResume();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPrepareDialog(int id, Dialog dialog)
    {
        if (!QtApplication.invokeDelegate(id, dialog).invoked)
            super.onPrepareDialog(id, dialog);
    }
    public void super_onPrepareDialog(int id, Dialog dialog)
    {
        super.onPrepareDialog(id, dialog);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onPrepareOptionsMenu(Menu menu)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(menu);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onPrepareOptionsMenu(menu);
    }
    public boolean super_onPrepareOptionsMenu(Menu menu)
    {
        return super.onPrepareOptionsMenu(menu);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onPreparePanel(int featureId, View view, Menu menu)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(featureId, view, menu);
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onPreparePanel(featureId, view, menu);
    }
    public boolean super_onPreparePanel(int featureId, View view, Menu menu)
    {
        return super.onPreparePanel(featureId, view, menu);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onRestart()
    {
        super.onRestart();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState)
    {
        if (!QtApplication.invokeDelegate(savedInstanceState).invoked)
            super.onRestoreInstanceState(savedInstanceState);
    }
    public void super_onRestoreInstanceState(Bundle savedInstanceState)
    {
        super.onRestoreInstanceState(savedInstanceState);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onResume()
    {
        super.onResume();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    public Object onRetainNonConfigurationInstance()
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate();
        if (res.invoked)
            return res.methodReturns;
        else
            return super.onRetainNonConfigurationInstance();
    }
    public Object super_onRetainNonConfigurationInstance()
    {
        return super.onRetainNonConfigurationInstance();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onSaveInstanceState(Bundle outState)
    {
        if (!QtApplication.invokeDelegate(outState).invoked)
            super.onSaveInstanceState(outState);
    }
    public void super_onSaveInstanceState(Bundle outState)
    {
        super.onSaveInstanceState(outState);

    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onSearchRequested()
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate();
        if (res.invoked)
            return (Boolean)res.methodReturns;
        else
            return super.onSearchRequested();
    }
    public boolean super_onSearchRequested()
    {
        return super.onSearchRequested();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onStart()
    {
        super.onStart();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onStop()
    {
        super.onStop();
        QtApplication.invokeDelegate();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onTitleChanged(CharSequence title, int color)
    {
        if (!QtApplication.invokeDelegate(title, color).invoked)
            super.onTitleChanged(title, color);
    }
    public void super_onTitleChanged(CharSequence title, int color)
    {
        super.onTitleChanged(title, color);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onTouchEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onTouchEvent, event);
        else
            return super.onTouchEvent(event);
    }
    public boolean super_onTouchEvent(MotionEvent event)
    {
        return super.onTouchEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onTrackballEvent(MotionEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onTrackballEvent != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onTrackballEvent, event);
        else
            return super.onTrackballEvent(event);
    }
    public boolean super_onTrackballEvent(MotionEvent event)
    {
        return super.onTrackballEvent(event);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onUserInteraction()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onUserInteraction();
    }
    public void super_onUserInteraction()
    {
        super.onUserInteraction();
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onUserLeaveHint()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onUserLeaveHint();
    }
    public void super_onUserLeaveHint()
    {
        super.onUserLeaveHint();
    }
    //---------------------------------------------------------------------------

    @Override
    public void onWindowAttributesChanged(LayoutParams params)
    {
        if (!QtApplication.invokeDelegate(params).invoked)
            super.onWindowAttributesChanged(params);
    }
    public void super_onWindowAttributesChanged(LayoutParams params)
    {
        super.onWindowAttributesChanged(params);
    }
    //---------------------------------------------------------------------------

    @Override
    public void onWindowFocusChanged(boolean hasFocus)
    {
        if (!QtApplication.invokeDelegate(hasFocus).invoked)
            super.onWindowFocusChanged(hasFocus);
    }
    public void super_onWindowFocusChanged(boolean hasFocus)
    {
        super.onWindowFocusChanged(hasFocus);
    }
    //---------------------------------------------------------------------------

    //////////////// Activity API 5 /////////////
//@ANDROID-5
    @Override
    public void onAttachedToWindow()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onAttachedToWindow();
    }
    public void super_onAttachedToWindow()
    {
        super.onAttachedToWindow();
    }
    //---------------------------------------------------------------------------

    @Override
    public void onBackPressed()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onBackPressed();
    }
    public void super_onBackPressed()
    {
        super.onBackPressed();
    }
    //---------------------------------------------------------------------------

    @Override
    public void onDetachedFromWindow()
    {
        if (!QtApplication.invokeDelegate().invoked)
            super.onDetachedFromWindow();
    }
    public void super_onDetachedFromWindow()
    {
        super.onDetachedFromWindow();
    }
    //---------------------------------------------------------------------------

    @Override
    public boolean onKeyLongPress(int keyCode, KeyEvent event)
    {
        if (QtApplication.m_delegateObject != null  && QtApplication.onKeyLongPress != null)
            return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onKeyLongPress, keyCode, event);
        else
            return super.onKeyLongPress(keyCode, event);
    }
    public boolean super_onKeyLongPress(int keyCode, KeyEvent event)
    {
        return super.onKeyLongPress(keyCode, event);
    }
    //---------------------------------------------------------------------------
//@ANDROID-5

//////////////// Activity API 8 /////////////
//@ANDROID-8
@Override
    protected Dialog onCreateDialog(int id, Bundle args)
    {
        QtApplication.InvokeResult res = QtApplication.invokeDelegate(id, args);
        if (res.invoked)
            return (Dialog)res.methodReturns;
        else
            return super.onCreateDialog(id, args);
    }
    public Dialog super_onCreateDialog(int id, Bundle args)
    {
        return super.onCreateDialog(id, args);
    }
    //---------------------------------------------------------------------------

    @Override
    protected void onPrepareDialog(int id, Dialog dialog, Bundle args)
    {
        if (!QtApplication.invokeDelegate(id, dialog, args).invoked)
            super.onPrepareDialog(id, dialog, args);
    }
    public void super_onPrepareDialog(int id, Dialog dialog, Bundle args)
    {
        super.onPrepareDialog(id, dialog, args);
    }
    //---------------------------------------------------------------------------
//@ANDROID-8
    //////////////// Activity API 11 /////////////

//@ANDROID-11
//QtCreator     @Override
//QtCreator     public boolean dispatchKeyShortcutEvent(KeyEvent event)
//QtCreator     {
//QtCreator         if (QtApplication.m_delegateObject != null  && QtApplication.dispatchKeyShortcutEvent != null)
//QtCreator             return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchKeyShortcutEvent, event);
//QtCreator         else
//QtCreator             return super.dispatchKeyShortcutEvent(event);
//QtCreator     }
//QtCreator     public boolean super_dispatchKeyShortcutEvent(KeyEvent event)
//QtCreator     {
//QtCreator         return super.dispatchKeyShortcutEvent(event);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//QtCreator 
//QtCreator     @Override
//QtCreator     public void onActionModeFinished(ActionMode mode)
//QtCreator     {
//QtCreator         if (!QtApplication.invokeDelegate(mode).invoked)
//QtCreator             super.onActionModeFinished(mode);
//QtCreator     }
//QtCreator     public void super_onActionModeFinished(ActionMode mode)
//QtCreator     {
//QtCreator         super.onActionModeFinished(mode);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//QtCreator 
//QtCreator     @Override
//QtCreator     public void onActionModeStarted(ActionMode mode)
//QtCreator     {
//QtCreator         if (!QtApplication.invokeDelegate(mode).invoked)
//QtCreator             super.onActionModeStarted(mode);
//QtCreator     }
//QtCreator     public void super_onActionModeStarted(ActionMode mode)
//QtCreator     {
//QtCreator         super.onActionModeStarted(mode);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//QtCreator 
//QtCreator     @Override
//QtCreator     public void onAttachFragment(Fragment fragment)
//QtCreator     {
//QtCreator         if (!QtApplication.invokeDelegate(fragment).invoked)
//QtCreator             super.onAttachFragment(fragment);
//QtCreator     }
//QtCreator     public void super_onAttachFragment(Fragment fragment)
//QtCreator     {
//QtCreator         super.onAttachFragment(fragment);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//QtCreator 
//QtCreator     @Override
//QtCreator     public View onCreateView(View parent, String name, Context context, AttributeSet attrs)
//QtCreator     {
//QtCreator         QtApplication.InvokeResult res = QtApplication.invokeDelegate(parent, name, context, attrs);
//QtCreator         if (res.invoked)
//QtCreator             return (View)res.methodReturns;
//QtCreator         else
//QtCreator             return super.onCreateView(parent, name, context, attrs);
//QtCreator     }
//QtCreator     public View super_onCreateView(View parent, String name, Context context,
//QtCreator             AttributeSet attrs) {
//QtCreator         return super.onCreateView(parent, name, context, attrs);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//QtCreator 
//QtCreator     @Override
//QtCreator     public boolean onKeyShortcut(int keyCode, KeyEvent event)
//QtCreator     {
//QtCreator         if (QtApplication.m_delegateObject != null  && QtApplication.onKeyShortcut != null)
//QtCreator             return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onKeyShortcut, keyCode,event);
//QtCreator         else
//QtCreator             return super.onKeyShortcut(keyCode, event);
//QtCreator     }
//QtCreator     public boolean super_onKeyShortcut(int keyCode, KeyEvent event)
//QtCreator     {
//QtCreator         return super.onKeyShortcut(keyCode, event);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//QtCreator 
//QtCreator     @Override
//QtCreator     public ActionMode onWindowStartingActionMode(Callback callback)
//QtCreator     {
//QtCreator         QtApplication.InvokeResult res = QtApplication.invokeDelegate(callback);
//QtCreator         if (res.invoked)
//QtCreator             return (ActionMode)res.methodReturns;
//QtCreator         else
//QtCreator             return super.onWindowStartingActionMode(callback);
//QtCreator     }
//QtCreator     public ActionMode super_onWindowStartingActionMode(Callback callback)
//QtCreator     {
//QtCreator         return super.onWindowStartingActionMode(callback);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//@ANDROID-11
    //////////////// Activity API 12 /////////////

//@ANDROID-12
//QtCreator     @Override
//QtCreator     public boolean dispatchGenericMotionEvent(MotionEvent ev)
//QtCreator     {
//QtCreator         if (QtApplication.m_delegateObject != null  && QtApplication.dispatchGenericMotionEvent != null)
//QtCreator             return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.dispatchGenericMotionEvent, ev);
//QtCreator         else
//QtCreator             return super.dispatchGenericMotionEvent(ev);
//QtCreator     }
//QtCreator     public boolean super_dispatchGenericMotionEvent(MotionEvent event)
//QtCreator     {
//QtCreator         return super.dispatchGenericMotionEvent(event);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//QtCreator 
//QtCreator     @Override
//QtCreator     public boolean onGenericMotionEvent(MotionEvent event)
//QtCreator     {
//QtCreator         if (QtApplication.m_delegateObject != null  && QtApplication.onGenericMotionEvent != null)
//QtCreator             return (Boolean) QtApplication.invokeDelegateMethod(QtApplication.onGenericMotionEvent, event);
//QtCreator         else
//QtCreator             return super.onGenericMotionEvent(event);
//QtCreator     }
//QtCreator     public boolean super_onGenericMotionEvent(MotionEvent event)
//QtCreator     {
//QtCreator         return super.onGenericMotionEvent(event);
//QtCreator     }
//QtCreator     //---------------------------------------------------------------------------
//@ANDROID-12

}
