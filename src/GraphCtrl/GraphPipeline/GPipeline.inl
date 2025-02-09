/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: GPipeline.inl
@Time: 2021/4/26 9:16 下午
@Desc:
***************************/

#ifndef CGRAPH_GPIPELINE_INL
#define CGRAPH_GPIPELINE_INL

#include <algorithm>

CGRAPH_NAMESPACE_BEGIN

template<typename T, std::enable_if_t<std::is_base_of_v<GElement, T>, int>>
CSTATUS GPipeline::registerGElement(GElementPtr *elementRef,
                                    const GElementPtrSet &dependElements,
                                    const std::string &name,
                                    int loop) {
    CGRAPH_FUNCTION_BEGIN
    CGRAPH_ASSERT_INIT(false)

    if constexpr (std::is_base_of_v<GGroup, T>) {
        /**
         * 如果是GGroup类型的信息，则：
         * 1，必须外部创建
         * 2，未被注册到其他的pipeline中
         */
        if ((*elementRef) != nullptr
            && (*elementRef)->param_manager_ != nullptr) {
            CGRAPH_ECHO("This group has already been registered to another pipeline.");
            return STATUS_ERR;
        }
    } else if constexpr (std::is_base_of_v<GNode, T>) {
        /**
         * 如果不是group信息的话，且属于element（包含node和aspect）
         * 则直接内部创建该信息
         */
        (*elementRef) = new(std::nothrow) T();
    } else if constexpr (std::is_base_of_v<GAdapter, T>) {
        (*elementRef) = new(std::nothrow) T();
    }

    CGRAPH_ASSERT_NOT_NULL(*elementRef)
    status = (*elementRef)->setElementInfo(dependElements, name, loop, this->param_manager_);
    CGRAPH_FUNCTION_CHECK_STATUS

    status = element_manager_->addElement(dynamic_cast<GElementPtr>(*elementRef));
    CGRAPH_FUNCTION_CHECK_STATUS
    element_repository_.insert(*elementRef);
    CGRAPH_FUNCTION_END
}


template<typename T, std::enable_if_t<std::is_base_of_v<GNode, T>, int>>
GNodePtr GPipeline::createGNode(const GNodeInfo &info) {
    CGRAPH_FUNCTION_BEGIN
    GNodePtr node = CGRAPH_SAFE_MALLOC_COBJECT(T);
    CGRAPH_ASSERT_NOT_NULL_RETURN_NULL(node)

    status = node->setElementInfo(info.dependence, info.name, info.loop, this->param_manager_);
    if (STATUS_OK != status) {
        CGRAPH_DELETE_PTR(node);
        return nullptr;
    }

    element_repository_.insert(node);
    return node;
}


template<typename T, std::enable_if_t<std::is_base_of_v<GGroup, T>, int>>
GGroupPtr GPipeline::createGGroup(const GElementPtrArr &elements,
                                  const GElementPtrSet &dependElements,
                                  const std::string &name,
                                  int loop) {
    CGRAPH_FUNCTION_BEGIN

    // 如果不是所有的都非空，则创建失败
    if (std::any_of(elements.begin(), elements.end(),
                    [](GElementPtr element) {
                        return (nullptr == element);
                    })) {
        return nullptr;
    }

    if (std::any_of(dependElements.begin(), dependElements.end(),
                    [](GElementPtr element) {
                        return (nullptr == element);
                    })) {
        return nullptr;
    }

    CGRAPH_ASSERT_NOT_NULL_RETURN_NULL(this->thread_pool_)    // 所有的pipeline必须有线程池

    GGroupPtr group = CGRAPH_SAFE_MALLOC_COBJECT(T)
    CGRAPH_ASSERT_NOT_NULL_RETURN_NULL(group)
    for (GElementPtr element : elements) {
        group->addElement(element);
    }

    if constexpr (std::is_same<GRegion, T>::value) {
        // 如果是GRegion类型，需要设置线程池信息。因为GRegion内部可能需要并行
        ((GRegion *)group)->setThreadPool(this->thread_pool_);
    }

    status = group->setElementInfo(dependElements, name, loop, nullptr);    // 注册group信息的时候，不能注册paramManager信息
    if (unlikely(STATUS_OK != status)) {
        CGRAPH_DELETE_PTR(group)
        return nullptr;
    }

    this->element_repository_.insert(group);
    return group;
}


template<typename T, std::enable_if_t<std::is_base_of_v<GParam, T>, int>>
GPipelinePtr GPipeline::addGParam(const std::string& key) {
    if (key.empty() || nullptr == param_manager_) {
        return nullptr;
    }

    /** 如果创建成功，返回自身（用于链式调用）
     * 如果失败，直接返回nullptr信息
     * */
    return (STATUS_OK == param_manager_->template create<T>(key)) ? this : nullptr;
}


template<typename TAspect, typename TParam,
        std::enable_if_t<std::is_base_of_v<GAspect, TAspect>, int>,
        std::enable_if_t<std::is_base_of_v<GAspectParam, TParam>, int>>
GPipeline* GPipeline::addGAspectBatch(const GElementPtrSet& elements, TParam* param) {
    const GElementPtrSet& curElements = elements.empty() ? element_repository_ : elements;
    for (auto element : curElements) {
        // 如果传入的为空，或者不是当前pipeline中的element，则不处理
        if (nullptr == element
            || (element_repository_.find(element) == element_repository_.end())) {
            CGRAPH_ECHO("[warning] input element [%p] is not suitable.", element);
            continue;
        }

        element->template addGAspect<TAspect, TParam>(param);
    }

    return this;
}

CGRAPH_NAMESPACE_END

#endif //CGRAPH_GPIPELINE_INL
