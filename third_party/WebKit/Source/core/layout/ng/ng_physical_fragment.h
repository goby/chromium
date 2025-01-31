// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NGPhysicalFragment_h
#define NGPhysicalFragment_h

#include "core/CoreExport.h"
#include "core/layout/ng/ng_physical_fragment_base.h"
#include "core/layout/ng/ng_units.h"
#include "platform/heap/Handle.h"

namespace blink {

class NGBlockNode;

class CORE_EXPORT NGPhysicalFragment final : public NGPhysicalFragmentBase {
 public:
  // This modifies the passed-in children vector.
  NGPhysicalFragment(
      NGPhysicalSize size,
      NGPhysicalSize overflow,
      HeapVector<Member<const NGPhysicalFragmentBase>>& children,
      HeapLinkedHashSet<WeakMember<NGBlockNode>>& out_of_flow_descendants,
      Vector<NGStaticPosition>& out_of_flow_positions,
      NGMarginStrut margin_strut);

  const HeapVector<Member<const NGPhysicalFragmentBase>>& Children() const {
    return children_;
  }

  NGMarginStrut MarginStrut() const { return margin_strut_; }

  DECLARE_TRACE_AFTER_DISPATCH();

 private:
  HeapVector<Member<const NGPhysicalFragmentBase>> children_;
  NGMarginStrut margin_strut_;
};

WILL_NOT_BE_EAGERLY_TRACED_CLASS(NGPhysicalFragment);

DEFINE_TYPE_CASTS(NGPhysicalFragment,
                  NGPhysicalFragmentBase,
                  fragment,
                  fragment->Type() == NGPhysicalFragmentBase::kFragmentBox,
                  fragment.Type() == NGPhysicalFragmentBase::kFragmentBox);

}  // namespace blink

#endif  // NGPhysicalFragment_h
