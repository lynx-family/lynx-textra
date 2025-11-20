package com.lynx.textra.skitydemo;

import android.content.Context;
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

public class SkityGLView extends GLSurfaceView implements GLSurfaceView.EGLConfigChooser {
  private final GLRenderer mRenderer;

  public SkityGLView(Context context, SkityRender skityRender) {
    super(context);

    setEGLConfigChooser(this);
    setEGLContextClientVersion(2);

    mRenderer = new GLRenderer(this, skityRender);

    setRenderer(mRenderer);
  }

  @Override
  public EGLConfig chooseConfig(EGL10 egl10, EGLDisplay eglDisplay) {
    int[] attrs = {
        EGL10.EGL_RED_SIZE,
        8,
        EGL10.EGL_GREEN_SIZE,
        8,
        EGL10.EGL_BLUE_SIZE,
        8,
        EGL10.EGL_ALPHA_SIZE,
        8,
        EGL10.EGL_DEPTH_SIZE,
        0,
        EGL10.EGL_STENCIL_SIZE,
        8,
        EGL10.EGL_SAMPLE_BUFFERS,
        1,
        EGL10.EGL_SAMPLES,
        4,
        EGL10.EGL_NONE,
    };

    EGLConfig[] configs = new EGLConfig[1];
    int[] numConfigs = new int[1];

    egl10.eglChooseConfig(eglDisplay, attrs, configs, 1, numConfigs);
    if (numConfigs[0] == 0) {
      // use fallback attribute to fit emulator EGL implement
      int[] attrs_back = {
          EGL10.EGL_RED_SIZE,
          8,
          EGL10.EGL_GREEN_SIZE,
          8,
          EGL10.EGL_BLUE_SIZE,
          8,
          EGL10.EGL_ALPHA_SIZE,
          8,
          EGL10.EGL_DEPTH_SIZE,
          0,
          EGL10.EGL_STENCIL_SIZE,
          8,
          EGL10.EGL_NONE,
      };

      egl10.eglChooseConfig(eglDisplay, attrs_back, configs, 1, numConfigs);
    }

    return configs[0];
  }

  public void onDestroy() {
    mRenderer.onDestroy();
  }
}
