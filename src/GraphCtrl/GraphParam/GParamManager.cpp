/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: GParamManager.cpp
@Time: 2021/6/11 6:59 下午
@Desc: 
***************************/

#include "GParamManager.h"

CGRAPH_NAMESPACE_BEGIN

GParamManager::GParamManager() {
    init();
}


GParamManager::~GParamManager() {
    deinit();
}


CSTATUS GParamManager::init() {
    CGRAPH_FUNCTION_BEGIN
    params_map_.clear();
    CGRAPH_FUNCTION_END
}


CSTATUS GParamManager::deinit() {
    CGRAPH_FUNCTION_BEGIN
    for (auto& param : params_map_) {
        CGRAPH_DELETE_PTR(param.second)
    }

    params_map_.clear();
    CGRAPH_FUNCTION_END
}


GParamPtr GParamManager::get(const std::string& key) {
    auto result = params_map_.find(key);
    if (result == params_map_.end()) {
        return nullptr;
    }

    return params_map_.find(key)->second;;
}


void GParamManager::reset() {
    for (auto cur : params_map_) {
        if (unlikely(!cur.second)) {
            continue;
        }

        cur.second->reset();
    }
}

CGRAPH_NAMESPACE_END
