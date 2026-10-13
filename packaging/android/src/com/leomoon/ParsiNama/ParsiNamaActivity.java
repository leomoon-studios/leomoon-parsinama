package com.leomoon.ParsiNama;

import android.os.Build;
import android.os.Bundle;
import android.window.OnBackInvokedCallback;
import android.window.OnBackInvokedDispatcher;
import org.qtproject.qt.android.bindings.QtActivity;

public class ParsiNamaActivity extends QtActivity {
    private native boolean handleNativeBack();
    private OnBackInvokedCallback backCallback;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Build.VERSION.SDK_INT >= 33) {
            backCallback = () -> {
                if (!dispatchBack())
                    moveTaskToBack(true);
            };
            getOnBackInvokedDispatcher().registerOnBackInvokedCallback(
                    OnBackInvokedDispatcher.PRIORITY_OVERLAY, backCallback);
        }
    }

    @Override
    protected void onDestroy() {
        if (Build.VERSION.SDK_INT >= 33 && backCallback != null)
            getOnBackInvokedDispatcher().unregisterOnBackInvokedCallback(backCallback);
        super.onDestroy();
    }

    private boolean dispatchBack() {
        try {
            return handleNativeBack();
        } catch (UnsatisfiedLinkError ignored) {
            return false;
        }
    }

    @Override
    public void onBackPressed() {
        if (!dispatchBack())
            super.onBackPressed();
    }
}
