/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp.toker@collabora.co.uk>
 * Copyright (C) 2008, Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/graphics/ImageSource.h"

#include "platform/graphics/DeferredImageDecoder.h"
#include "platform/image-decoders/ImageDecoder.h"
#include "third_party/skia/include/core/SkImage.h"

namespace blink {

ImageSource::ImageSource()
    : m_decoderColorBehavior(ColorBehavior::transformToGlobalTarget()) {}

ImageSource::~ImageSource() {}

size_t ImageSource::clearCacheExceptFrame(size_t clearExceptFrame) {
  return m_decoder ? m_decoder->clearCacheExceptFrame(clearExceptFrame) : 0;
}

PassRefPtr<SharedBuffer> ImageSource::data() {
  return m_decoder ? m_decoder->data() : nullptr;
}

bool ImageSource::setData(PassRefPtr<SharedBuffer> passData,
                          bool allDataReceived) {
  RefPtr<SharedBuffer> data = passData;
  m_allDataReceived = allDataReceived;

  if (m_decoder) {
    m_decoder->setData(data.release(), allDataReceived);
    // If the decoder is pre-instantiated, it means we've already validated the
    // data/signature at some point.
    return true;
  }

  m_decoder = DeferredImageDecoder::create(data, allDataReceived,
                                           ImageDecoder::AlphaPremultiplied,
                                           m_decoderColorBehavior);

  // Insufficient data is not a failure.
  return m_decoder || !ImageDecoder::hasSufficientDataToSniffImageType(*data);
}

String ImageSource::filenameExtension() const {
  return m_decoder ? m_decoder->filenameExtension() : String();
}

bool ImageSource::isSizeAvailable() {
  return m_decoder && m_decoder->isSizeAvailable();
}

bool ImageSource::hasColorProfile() const {
  return m_decoder && m_decoder->hasEmbeddedColorSpace();
}

IntSize ImageSource::size(
    RespectImageOrientationEnum shouldRespectOrientation) const {
  return frameSizeAtIndex(0, shouldRespectOrientation);
}

IntSize ImageSource::frameSizeAtIndex(
    size_t index,
    RespectImageOrientationEnum shouldRespectOrientation) const {
  if (!m_decoder)
    return IntSize();

  IntSize size = m_decoder->frameSizeAtIndex(index);
  if ((shouldRespectOrientation == RespectImageOrientation) &&
      m_decoder->orientationAtIndex(index).usesWidthAsHeight())
    return size.transposedSize();

  return size;
}

bool ImageSource::getHotSpot(IntPoint& hotSpot) const {
  return m_decoder ? m_decoder->hotSpot(hotSpot) : false;
}

int ImageSource::repetitionCount() {
  return m_decoder ? m_decoder->repetitionCount() : cAnimationNone;
}

size_t ImageSource::frameCount() const {
  return m_decoder ? m_decoder->frameCount() : 0;
}

sk_sp<SkImage> ImageSource::createFrameAtIndex(
    size_t index,
    const ColorBehavior& colorBehavior) {
  if (!m_decoder)
    return nullptr;

  if (colorBehavior != m_decoderColorBehavior) {
    m_decoder = DeferredImageDecoder::create(data(), m_allDataReceived,
                                             ImageDecoder::AlphaPremultiplied,
                                             colorBehavior);
    m_decoderColorBehavior = colorBehavior;
    // The data has already been validated, so changing the color behavior
    // should always result in a valid decoder.
    DCHECK(m_decoder);
  }

  return m_decoder->createFrameAtIndex(index);
}

float ImageSource::frameDurationAtIndex(size_t index) const {
  if (!m_decoder)
    return 0;

  // Many annoying ads specify a 0 duration to make an image flash as quickly as
  // possible. We follow Firefox's behavior and use a duration of 100 ms for any
  // frames that specify a duration of <= 10 ms. See <rdar://problem/7689300>
  // and <http://webkit.org/b/36082> for more information.
  const float duration = m_decoder->frameDurationAtIndex(index) / 1000.0f;
  if (duration < 0.011f)
    return 0.100f;
  return duration;
}

ImageOrientation ImageSource::orientationAtIndex(size_t index) const {
  return m_decoder ? m_decoder->orientationAtIndex(index)
                   : DefaultImageOrientation;
}

bool ImageSource::frameHasAlphaAtIndex(size_t index) const {
  return !m_decoder || m_decoder->frameHasAlphaAtIndex(index);
}

bool ImageSource::frameIsCompleteAtIndex(size_t index) const {
  return m_decoder && m_decoder->frameIsCompleteAtIndex(index);
}

size_t ImageSource::frameBytesAtIndex(size_t index) const {
  return m_decoder ? m_decoder->frameBytesAtIndex(index) : 0;
}

}  // namespace blink
