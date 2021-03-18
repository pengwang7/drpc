/*
 * MIT License
 *
 * Copyright (c) 2020 pengwang7
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ANY_CAST_H__
#define __ANY_CAST_H__

#include <utility>
#include <typeinfo>
#include <type_traits>

class any final {
public:
    any() : content(nullptr) {}

    any(const any& rhs) : content(rhs.content ? rhs.content->clone() : nullptr) {}

    template <typename ValueType>
    any(const ValueType& value) : content(new holder<ValueType>(value)) {};

    ~any() {
        delete content;
    }

public:
    bool empty() const {
        return !content;
    }

    const std::type_info& type() const {
        return content ? content->type() : typeid(void);
    }

    any& swap(any& rhs) {
        std::swap(content, rhs.content);
        return *this;
    }

    any& operator=(any rhs) {
        rhs.swap(*this);
        return *this;
    }

    template <typename ValueType>
    any& operator=(const ValueType& rhs) {
        any(rhs).swap(*this);
        return *this;
    }

public:
    class placeholder {
    public:
        virtual ~placeholder() {}

        virtual placeholder* clone() const = 0;

        virtual const std::type_info& type() const = 0;

    };

    template <typename ValueType>
    class holder : public placeholder {
    public:
        holder(const ValueType& value) : held(value) {}

        placeholder* clone() const override {
            return new holder(held);
        }

        const std::type_info & type() const override {
            return typeid(ValueType);
        }

    private:
        // Intentionally left unimplemented.
        holder& operator=(const holder&);
    public:
        ValueType held;
    };

    placeholder* content;
};

class bad_any_cast : public std::bad_cast {
public:
    const char* what() const throw() override {
        return "any_cast failed.";
    }
};

template <typename ValueType>
ValueType* any_cast(any* operand) {
    return operand && operand->type() == typeid(ValueType)
           ? &static_cast<any::holder<ValueType> *>(operand->content)->held : nullptr;
}

template <typename ValueType>
inline const ValueType* any_cast(const any* operand) {
    return any_cast<ValueType>(const_cast<any*>(operand));
}

template< class T >
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename ValueType>
ValueType any_cast(any& operand) {
    using nonref = typename std::remove_reference<ValueType>::type;

    nonref* value = any_cast<nonref>(&operand);
    if (!value) {
        throw bad_any_cast();
    }

    return *value;
}

template <typename ValueType>
inline ValueType any_cast(const any& operand) {
    using nonref = typename std::remove_reference<ValueType>::type;

    return any_cast<const nonref&>(const_cast<any&>(operand));
}

#endif // __ANY_CAST_H__
