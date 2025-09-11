// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { DrawContext, FrameNode } from '@kit.ArkUI';
import { drawing } from '@kit.ArkGraphics2D';

export const initLayoutContext: (layoutContext: object, type: number) => void;

export const buildCanvas: (layoutContext: object, width: number, height: number) => void;

export const buildParagraph: (layoutContext: object, content: string) => void;

export const layoutParagraph: (layoutContext: object, width: number) => void;

export const drawParagraph: (canvas: DrawContext, layoutContext: object, x: number, y: number) => void;

