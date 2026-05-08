#ifndef DIX_CLASS_HPP
#define DIX_CLASS_HPP

#define DIX_DISABLE_COPY(ClassName) \
    ClassName(const ClassName&) = delete; \
    ClassName& operator=(const ClassName&) = delete;

#define DIX_DISABLE_MOVE(ClassName) \
    ClassName(ClassName&&) = delete; \
    ClassName& operator=(ClassName&&) = delete;

#define DIX_DISABLE_COPY_AND_MOVE(ClassName) \
    DIX_DISABLE_COPY(ClassName) \
    DIX_DISABLE_MOVE(ClassName)

#define DIX_DISABLE_EVERYTHING(ClassName) \
    DIX_DISABLE_COPY_AND_MOVE(ClassName) \
    ClassName() = delete; \
    ~ClassName() = delete;
    

#endif // DIX_CLASS_HPP