// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.vr_shell;

import android.app.Activity;
import android.os.StrictMode;

import org.chromium.base.Log;
import org.chromium.base.annotations.UsedByReflection;

/**
 * Builder class to create all VR related classes. These VR classes are behind the same build time
 * flag as this class. So no reflection is necessary when create them.
 */
@UsedByReflection("VrShellDelegate.java")
public class VrClassesBuilderImpl implements VrClassesBuilder {
    private static final String TAG = "VrClassesBuilderImpl";
    private final Activity mActivity;

    @UsedByReflection("VrShellDelegate.java")
    public VrClassesBuilderImpl(Activity activity) {
        mActivity = activity;
    }

    @Override
    public NonPresentingGvrContext createNonPresentingGvrContext() {
        StrictMode.ThreadPolicy oldPolicy = StrictMode.allowThreadDiskReads();
        try {
            return new NonPresentingGvrContextImpl(mActivity);
        } catch (Exception ex) {
            Log.e(TAG, "Unable to instantiate NonPresentingGvrContextImpl", ex);
            return null;
        } finally {
            StrictMode.setThreadPolicy(oldPolicy);
        }
    }

    @Override
    public VrShell createVrShell() {
        StrictMode.ThreadPolicy oldPolicy = StrictMode.allowThreadDiskReads();
        try {
            return new VrShellImpl(mActivity);
        } catch (Exception ex) {
            Log.e(TAG, "Unable to instantiate VrShellImpl", ex);
            return null;
        } finally {
            StrictMode.setThreadPolicy(oldPolicy);
        }
    }

    @Override
    public VrDaydreamApi createVrDaydreamApi() {
        return new VrDaydreamApiImpl(mActivity);
    }

    @Override
    public VrCoreVersionChecker createVrCoreVersionChecker() {
        return new VrCoreVersionCheckerImpl();
    }
}
