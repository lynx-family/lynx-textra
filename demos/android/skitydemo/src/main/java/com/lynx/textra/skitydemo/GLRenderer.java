package com.lynx.textra.skitydemo;

import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class GLRenderer implements GLSurfaceView.Renderer {
  private final GLSurfaceView mView;
  private final SkityRender mSkityRender;

  public GLRenderer(GLSurfaceView view, SkityRender skityRender) {
    mView = view;
    mSkityRender = skityRender;
  }

  @Override
  public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
    mSkityRender.init(mView.getWidth(), mView.getHeight(),
        (int) mView.getContext().getResources().getDisplayMetrics().density, mView.getContext());
  }

  @Override
  public void onSurfaceChanged(GL10 gl10, int i, int i1) {}

  @Override
  public void onDrawFrame(GL10 gl10) {
    mSkityRender.draw();
  }

  public void onDestroy() {
    mSkityRender.destroy();
  }
}
