// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

import { DelegateAttribute, Paragraph, ParagraphStyle, RunStyle } from '../../../ets/Paragraph'
import { TextNode } from '../../../ets/TTTextNode';
import { ComponentContent, FrameNode } from '@kit.ArkUI';

export const bindNativeParagraph: (self: Object) => void;

export const setParagraphStyle: (self: Object, para_style: ParagraphStyle) => void;

export const addTextRun: (self: Paragraph, content: string, run_style: RunStyle) => void;

export const addFrameNode: (self: Paragraph, node: FrameNode, width: number,
  height: number) => void;

export const createNativeTextNode: (content: Object) => void;

export const destroyNativeTextNode: () => void;

export const appendParagraphToTextNode: (self: TextNode, para: Paragraph) => void;

