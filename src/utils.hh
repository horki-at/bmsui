#ifndef INCLUDED_UTILS_
#define INCLUDED_UTILS_

// Disables default copy/move constructors and assignments.
#define DISABLE_CPY_MV(ClassName)                                              \
ClassName(ClassName const &) = delete;                                         \
ClassName(ClassName &&) = delete;                                              \
ClassName &operator=(ClassName const &) = delete;                               \
ClassName &operator=(ClassName &&) = delete

#endif // INCLUDED_UTILS_
