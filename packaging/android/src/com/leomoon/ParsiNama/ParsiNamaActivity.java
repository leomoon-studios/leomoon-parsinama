package com.leomoon.ParsiNama;

import org.qtproject.qt.android.bindings.QtActivity;

public class ParsiNamaActivity extends QtActivity {
    private native boolean handleNativeBack();

    @Override
    public void onBackPressed() {
        try {
            if (handleNativeBack())
                return;
        } catch (UnsatisfiedLinkError ignored) {
            // Qt may still be starting when the user presses Back.
        }
        super.onBackPressed();
    }
}
