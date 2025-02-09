/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: GAspectObject.inl
@Time: 2021/10/2 9:46 下午
@Desc:
***************************/

#ifndef CGRAPH_GASPECTOBJECT_INL
#define CGRAPH_GASPECTOBJECT_INL

CGRAPH_NAMESPACE_BEGIN

template <typename T,
          std::enable_if_t<std::is_base_of_v<GAspectParam, T>, int>>
GAspectObjectPtr GAspectObject::setParam(T* param) {
    /** 传入的param可以为空 */
    if (param) {
        CGRAPH_DELETE_PTR(param_)
        param_ = CGRAPH_SAFE_MALLOC_COBJECT(T);
        param_->clone(static_cast<T *>(param));
    }

    return this;
}


template <typename T,
          std::enable_if_t<std::is_base_of_v<GAspectParam, T>, int>>
T* GAspectObject::getParam() {
    CGRAPH_ASSERT_NOT_NULL_RETURN_NULL(param_)

    T* param = nullptr;
    if ((typeid(*param_).name() == typeid(T).name())) {
        // 如果类型相同才可以获取成功，否则直接返回nullptr
        param = dynamic_cast<T *>(this->param_);
    }
    return param;
}


template <typename T,
          std::enable_if_t<std::is_base_of_v<GParam, T>, int>>
T* GAspectObject::getPipelineParam(const std::string& key) {
    CGRAPH_ASSERT_NOT_NULL_RETURN_NULL(pipeline_param_manager_)

    T* ptr = dynamic_cast<T *>(this->pipeline_param_manager_->get(key));
    return ptr;
}

CGRAPH_NAMESPACE_END

#endif //CGRAPH_GASPECTOBJECT_INL
