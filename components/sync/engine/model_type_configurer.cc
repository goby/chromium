// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/sync/engine/model_type_configurer.h"

namespace syncer {

// static
ModelTypeSet ModelTypeConfigurer::GetDataTypesInState(
    DataTypeConfigState state,
    const DataTypeConfigStateMap& state_map) {
  ModelTypeSet types;
  for (DataTypeConfigStateMap::const_iterator type_it = state_map.begin();
       type_it != state_map.end(); ++type_it) {
    if (type_it->second == state)
      types.Put(type_it->first);
  }
  return types;
}

// static
void ModelTypeConfigurer::SetDataTypesState(DataTypeConfigState state,
                                            ModelTypeSet types,
                                            DataTypeConfigStateMap* state_map) {
  for (ModelTypeSet::Iterator it = types.First(); it.Good(); it.Inc()) {
    (*state_map)[it.Get()] = state;
  }
}

}  // namespace syncer
