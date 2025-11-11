package com.lynx.textra;

import android.content.Context;
import android.os.Build;
import android.provider.Settings;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;

public interface PlatformFontCaps {
  public static final String DEFAULT_FAMILY_NAME = "sans-serif";

  public static int MaxVariation = 1000;

  public static int MinVariation = 100;

  /**
   * @param old original weight value.
   * @return Return a new weight value after applying platform-specific weight adjustment logic.
   */
  short adjust(short old);

  /**
   * Since many domestic Android devices include multiple vendor-specific fonts in addition to
   * the AOSP default fonts, there are usually two or more default fonts available for users to
   * choose from.
   *
   * @return Return the default font family name selected by the user.
   */
  String defaultFamilyName();

  default short clamp(int weight) {
    if (weight >= MaxVariation) {
      weight = MaxVariation;
    }
    if (weight <= MinVariation) {
      weight = MinVariation;
    }
    return (short) weight;
  }

  public class Factory {
    public static PlatformFontCaps getStrategy(Context context) {
      if ("oppo".equalsIgnoreCase(Build.BRAND)) {
        return new OppoFontCaps(context);
      } else if ("vivo".equalsIgnoreCase(Build.BRAND)) {
        return new VivoFontCaps();
      } else if ("xiaomi".equalsIgnoreCase(Build.BRAND)) {
        return new XiaomiFontCaps(context);
      } else if ("huawei".equalsIgnoreCase(Build.BRAND)) {
        return new HuaweiFontCaps(context);
      } else if ("honor".equalsIgnoreCase(Build.BRAND)) {
        return new HonorFontCaps(context);
      } else {
        return new DefaultFontCaps();
      }
    }
  }
}

class OppoFontCaps implements PlatformFontCaps {
  private static int FONT_VARIATION_STEP = 10;

  private final Map<Integer, Integer> FONT_WEIGHT_CAST_WGHT;
  private final Context mContext;

  public OppoFontCaps(Context context) {
    mContext = context;
    FONT_WEIGHT_CAST_WGHT = new LinkedHashMap<>();
    if (Build.VERSION.SDK_INT >= 35) { // Build.VERSION_CODES.VANILLA_ICE_CREAM
      // Use on Android 15 and above
      FONT_WEIGHT_CAST_WGHT.put(100, 100);
      FONT_WEIGHT_CAST_WGHT.put(200, 200);
      FONT_WEIGHT_CAST_WGHT.put(300, 300);
      FONT_WEIGHT_CAST_WGHT.put(400, 400);
      FONT_WEIGHT_CAST_WGHT.put(500, 500);
      FONT_WEIGHT_CAST_WGHT.put(600, 600);
      FONT_WEIGHT_CAST_WGHT.put(700, 700);
      FONT_WEIGHT_CAST_WGHT.put(800, 800);
      FONT_WEIGHT_CAST_WGHT.put(900, 900);
      FONT_WEIGHT_CAST_WGHT.put(1000, 1000);
    } else if (Build.VERSION.SDK_INT >= 34) {
      // Use on Android 14
      FONT_WEIGHT_CAST_WGHT.put(100, 100);
      FONT_WEIGHT_CAST_WGHT.put(200, 200);
      FONT_WEIGHT_CAST_WGHT.put(300, 300);
      FONT_WEIGHT_CAST_WGHT.put(400, 550);
      FONT_WEIGHT_CAST_WGHT.put(500, 700);
      FONT_WEIGHT_CAST_WGHT.put(600, 900);
      FONT_WEIGHT_CAST_WGHT.put(700, 1000);
      FONT_WEIGHT_CAST_WGHT.put(800, 1000);
      FONT_WEIGHT_CAST_WGHT.put(900, 1000);
      FONT_WEIGHT_CAST_WGHT.put(1000, 1000);
    } else {
      // Use on Android < 14
      FONT_WEIGHT_CAST_WGHT.put(100, 200);
      FONT_WEIGHT_CAST_WGHT.put(200, 250);
      FONT_WEIGHT_CAST_WGHT.put(300, 350);
      FONT_WEIGHT_CAST_WGHT.put(400, 550);
      FONT_WEIGHT_CAST_WGHT.put(500, 700);
      FONT_WEIGHT_CAST_WGHT.put(600, 700);
      FONT_WEIGHT_CAST_WGHT.put(700, 700);
      FONT_WEIGHT_CAST_WGHT.put(800, 850);
      FONT_WEIGHT_CAST_WGHT.put(900, 1000);
      FONT_WEIGHT_CAST_WGHT.put(1000, 1000);
    }
  }

  private int ToUserWeight(int font_weight) {
    Integer prev = null;
    Integer prevKey = null;

    for (Map.Entry<Integer, Integer> e : FONT_WEIGHT_CAST_WGHT.entrySet()) {
      int value = e.getValue();
      if (font_weight == value) {
        return e.getKey();
      }
      if (prev == null && font_weight < value) {
        return e.getKey();
      } else if (prev != null && font_weight > prev && font_weight < value) {
        float ratio = 1.f * (font_weight - prev) / (value - prev);
        int lerp_key = prevKey + (int) (ratio * (e.getKey() - prevKey));
        return lerp_key;
      }
      prev = value;
      prevKey = e.getKey();
    }
    return prevKey;
  }

  @Override
  public short adjust(short old) {
    int mFontVariationSettings =
        Settings.System.getInt(mContext.getContentResolver(), "font_variation_settings", 550);
    // An integer value where, when represented in hexadecimal, the 4th digit from the right
    // indicates the status
    int mFontVariationStatus = (int) ((mFontVariationSettings) & 0x00000f000) >> 12;

    if (mFontVariationStatus == 1) {
      // The first three digits represent the weight
      int mFontVariationValues = (int) ((mFontVariationSettings) & 0x000000fff);
      mFontVariationValues = mFontVariationValues / FONT_VARIATION_STEP * FONT_VARIATION_STEP;
      Integer wghtFromWeight = FONT_WEIGHT_CAST_WGHT.get((int) old);
      mFontVariationValues =
          mFontVariationValues + (wghtFromWeight == null ? old - 550 : wghtFromWeight - 550);

      return (short) ToUserWeight(clamp(mFontVariationValues));
    }
    return old;
  }

  @Override
  public String defaultFamilyName() {
    String familyName =
        Settings.System.getString(mContext.getContentResolver(), "current_typeface_name");
    if (Objects.equals(familyName, "OPPO Sans")) {
      return "sys-sans-en";
    } else if (Objects.equals(familyName, "One Sans")) {
      return "op-sans-en";
    }
    return DEFAULT_FAMILY_NAME;
  }
}

class VivoFontCaps implements PlatformFontCaps {
  @Override
  public short adjust(short old) {
    int scaledWeight = TTTextUtils.nativeGetSystemPropInt("persist.system.vivo.fontsize");
    if (scaledWeight <= 0) {
      scaledWeight = 550;
    }
    float scale = scaledWeight / 550.f;
    int weight = (int) (scale * old);
    return clamp(weight);
  }

  @Override
  public String defaultFamilyName() {
    return DEFAULT_FAMILY_NAME;
  }
}

class XiaomiFontCaps implements PlatformFontCaps {
  public static final int WEIGHT_IDX_THIN = 0;
  public static final int WEIGHT_IDX_EXTRA_LIGHT = 1;
  public static final int WEIGHT_IDX_LIGHT = 2;
  public static final int WEIGHT_IDX_NORMAL = 3;
  public static final int WEIGHT_IDX_REGULAR = 4;
  public static final int WEIGHT_IDX_MEDIUM = 5;
  public static final int WEIGHT_IDX_DEMI_BOLD = 6;
  public static final int WEIGHT_IDX_SEMI_BOLD = 7;
  public static final int WEIGHT_IDX_BOLD = 8;
  public static final int WEIGHT_IDX_HEAVY = 9;

  public static final int[][] NORAML_RULES =
      new int[][] {new int[] {WEIGHT_IDX_THIN, WEIGHT_IDX_MEDIUM},
          new int[] {WEIGHT_IDX_THIN, WEIGHT_IDX_MEDIUM},
          new int[] {WEIGHT_IDX_EXTRA_LIGHT, WEIGHT_IDX_DEMI_BOLD},
          new int[] {WEIGHT_IDX_LIGHT, WEIGHT_IDX_DEMI_BOLD},
          new int[] {WEIGHT_IDX_LIGHT, WEIGHT_IDX_SEMI_BOLD},
          new int[] {WEIGHT_IDX_NORMAL, WEIGHT_IDX_BOLD},
          new int[] {WEIGHT_IDX_REGULAR, WEIGHT_IDX_BOLD},
          new int[] {WEIGHT_IDX_MEDIUM, WEIGHT_IDX_HEAVY},
          new int[] {WEIGHT_IDX_DEMI_BOLD, WEIGHT_IDX_HEAVY},
          new int[] {WEIGHT_IDX_SEMI_BOLD, WEIGHT_IDX_HEAVY}};

  public static int[] WEIGHT_ARRAY = new int[] {100, 200, 300, 350, 400, 500, 700, 800, 900, 950};
  public static int[] MIUI_WGHT = new int[] {150, 200, 250, 305, 330, 380, 450, 520, 630, 700};

  private Map<Integer, Integer> mUserToMIUI = new LinkedHashMap<>();
  ;

  public int mFontScale = 50;
  private final Context mContext;

  public XiaomiFontCaps(Context context) {
    mContext = context;

    String value = null;
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
      value = Settings.Global.getString(context.getContentResolver(), "key_miui_font_var_weight");
    }
    if (value != null) {
      String[] elements = value.split(",");
      if (elements.length != MIUI_WGHT.length) {
        return;
      }
      for (int i = 0; i < elements.length; i++) {
        MIUI_WGHT[i] = Integer.parseInt(elements[i]);
      }
    }
    for (int i = 0; i < WEIGHT_ARRAY.length; i++) {
      mUserToMIUI.put(WEIGHT_ARRAY[i], MIUI_WGHT[i]);
    }
    mFontScale =
        Settings.System.getInt(context.getContentResolver(), "key_miui_font_weight_scale", 50);
  }

  public int getWeightIdx(int weight) {
    if (weight >= WEIGHT_ARRAY[WEIGHT_IDX_HEAVY]) {
      return WEIGHT_IDX_HEAVY;
    }
    for (int i = 0; i < WEIGHT_ARRAY.length; i++) {
      if (weight <= WEIGHT_ARRAY[i]) {
        return i;
      }
    }
    return WEIGHT_IDX_REGULAR;
  }

  public int getWght(int weight) {
    int weightIdx = getWeightIdx(weight);

    int startWght = MIUI_WGHT[NORAML_RULES[weightIdx][0]];
    int midWght = MIUI_WGHT[weightIdx];
    int endWght = MIUI_WGHT[NORAML_RULES[weightIdx][1]];

    int wght = midWght;
    if (mFontScale < 50) {
      float t = mFontScale / 50f;
      wght = (int) ((1f - t) * startWght + t * midWght);
    } else {
      float t = (mFontScale - 50f) / 50;
      wght = (int) ((1f - t) * midWght + t * endWght);
    }
    return wght;
  }

  private int ToUserWeight(int font_weight) {
    Integer prev = null;
    Integer prevKey = null;

    for (Map.Entry<Integer, Integer> e : mUserToMIUI.entrySet()) {
      int value = e.getValue();
      if (font_weight == value) {
        return e.getKey();
      }
      if (prev == null && font_weight < value) {
        return e.getKey();
      } else if (prev != null && font_weight > prev && font_weight < value) {
        float ratio = 1.f * (font_weight - prev) / (value - prev);
        int lerp_key = prevKey + (int) (ratio * (e.getKey() - prevKey));
        return lerp_key;
      }
      prev = value;
      prevKey = e.getKey();
    }
    return prevKey;
  }

  @Override
  public short adjust(short old) {
    int weight = ToUserWeight(getWght(old));
    return clamp(weight);
  }

  @Override
  public String defaultFamilyName() {
    return DEFAULT_FAMILY_NAME;
  }
}

// For huawei, excluding honor
class HuaweiFontCaps implements PlatformFontCaps {
  private final Context mContext;

  public HuaweiFontCaps(Context context) {
    mContext = context;
  }

  @Override
  public short adjust(short old) {
    float font_weight_scale =
        Settings.System.getFloat(mContext.getContentResolver(), "font_weight_scale", 1);
    int weight = (int) (font_weight_scale * old);
    return clamp(weight);
  }

  @Override
  public String defaultFamilyName() {
    return DEFAULT_FAMILY_NAME;
  }
}

// For honor
class HonorFontCaps implements PlatformFontCaps {
  private final Context mContext;

  public HonorFontCaps(Context context) {
    mContext = context;
  }

  @Override
  public short adjust(short old) {
    // Only applies to Honor’s own fonts, not to AOSP default fonts.
    int device_font_weight_scale =
        Settings.Secure.getInt(mContext.getContentResolver(), "font_weight_scale", 100);
    float font_weight_scale = device_font_weight_scale / 100.f;
    int weight = (int) (font_weight_scale * old);
    return clamp(weight);
  }

  @Override
  public String defaultFamilyName() {
    return DEFAULT_FAMILY_NAME;
  }
}

class DefaultFontCaps implements PlatformFontCaps {
  @Override
  public short adjust(short old) {
    return old;
  }

  @Override
  public String defaultFamilyName() {
    return DEFAULT_FAMILY_NAME;
  }
}
