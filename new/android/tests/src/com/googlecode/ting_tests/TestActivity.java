package com.googlecode.ting_tests;

import android.app.NativeActivity;

public class TestActivity extends NativeActivity {

static {

System.loadLibrary("gnustl_shared");
System.loadLibrary("ting_tests");
}

}
