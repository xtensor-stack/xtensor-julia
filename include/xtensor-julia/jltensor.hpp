/***************************************************************************
* Copyright (c) Johan Mabille and Sylvain Corlay                           *
* Copyright (c) QuantStack                                                 *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XTENSOR_JULIA_HPP
#define XTENSOR_JULIA_HPP

#include <algorithm>
#include <array>
#include <complex>
#include <cstddef>
#include <exception>

#include "xtensor/xbuffer_adaptor.hpp"
#include "xtensor/xsemantic.hpp"

#include "jlcxx/type_conversion.hpp"

#include "jlcontainer.hpp"

namespace xt
{

    /************************
     * jltensor declaration *
     ************************/

    template <class T, std::size_t N>
    class jltensor;

    template <class T, std::size_t N>
    struct xiterable_inner_types<jltensor<T, N>>
        : xcontainer_iterable_types<jltensor<T, N>>
    {
    };

    template <class T, std::size_t N>
    struct xcontainer_inner_types<jltensor<T, N>>
    {
        using storage_type = xbuffer_adaptor<jlcxx::static_julia_type<T>*>;
        using reference = typename storage_type::reference;
        using const_reference = typename storage_type::const_reference;
        using size_type = typename storage_type::size_type;
        using shape_type = std::array<std::size_t, N>;
        using strides_type = std::array<std::ptrdiff_t, N>;
        using backstrides_type = strides_type;
        using inner_shape_type = shape_type;
        using inner_strides_type = strides_type;
        using inner_backstrides_type = backstrides_type;
        using temporary_type = jltensor<T, N>;
        static constexpr layout_type layout = layout_type::column_major;
    };

    template <class T, std::size_t N>
    struct xcontainer_inner_types<jltensor<std::complex<T>, N>>
    {
        using storage_type = xbuffer_adaptor<std::complex<T>*>;
        using reference = typename storage_type::reference;
        using const_reference = typename storage_type::const_reference;
        using size_type = typename storage_type::size_type;
        using shape_type = std::array<std::size_t, N>;
        using strides_type = std::array<std::ptrdiff_t, N>;
        using backstrides_type = strides_type;
        using inner_shape_type = shape_type;
        using inner_strides_type = strides_type;
        using inner_backstrides_type = backstrides_type;
        using temporary_type = jltensor<T, N>;
        static constexpr layout_type layout = layout_type::column_major;
    };

    template <class T, std::size_t N>
    class jltensor : public jlcontainer<jltensor<T, N>>,
                     public xcontainer_semantic<jltensor<T, N>>
    {
    public:

        using self_type = jltensor<T, N>;
        using semantic_base = xcontainer_semantic<self_type>;
        using base_type = jlcontainer<self_type>;
        using storage_type = typename base_type::storage_type;
        using value_type = typename base_type::value_type;
        using reference = typename base_type::reference;
        using const_reference = typename base_type::const_reference;
        using pointer = typename base_type::pointer;
        using size_type = typename base_type::size_type;
        using shape_type = typename base_type::shape_type;
        using strides_type = typename base_type::strides_type;
        using backstrides_type = typename base_type::backstrides_type;
        using inner_shape_type = typename base_type::inner_shape_type;
        using inner_strides_type = typename base_type::inner_strides_type;
        using inner_backstrides_type = typename base_type::inner_backstrides_type;

        jltensor();
        jltensor(nested_initializer_list_t<T, N> t);

        explicit jltensor(const shape_type& shape);
        explicit jltensor(const shape_type& shape, const_reference value);
        jltensor(jl_array_t* jl);

        template <class S = shape_type>
        static jltensor from_shape(S&& shape);

        jltensor(const self_type&);
        self_type& operator=(const self_type&);

        jltensor(self_type&&) = default;
        self_type& operator=(self_type&&) = default;

        template <class E>
        jltensor(const xexpression<E>& e);

        template <class E>
        self_type& operator=(const xexpression<E>& e);

        using base_type::begin;
        using base_type::end;

    private:

        inner_shape_type m_shape;
        inner_strides_type m_strides;
        inner_backstrides_type m_backstrides;
        storage_type m_data;

        void init_tensor(const shape_type& shape);
        void init_from_julia();

        inner_shape_type& shape_impl() noexcept;
        const inner_shape_type& shape_impl() const noexcept;
        inner_strides_type& strides_impl() noexcept;
        const inner_strides_type& strides_impl() const noexcept;
        inner_backstrides_type& backstrides_impl() noexcept;
        const inner_backstrides_type& backstrides_impl() const noexcept;

        storage_type& storage_impl() noexcept;
        const storage_type& storage_impl() const noexcept;

        friend class xcontainer<jltensor<T, N>>;
        friend class jlcontainer<jltensor<T, N>>;
    };

    /***************************
     * jltensor implementation *
     ***************************/

    /**
     * @name Constructors
     */
    //@{
    /**
     * Allocates a jltensor that holds 1 element.
     */
    template <class T, std::size_t N>
    inline jltensor<T, N>::jltensor()
        : base_type()
    {
        m_shape = xtl::make_sequence<shape_type>(N, size_type(1));
        xt::compute_strides(m_shape, layout_type::column_major, m_strides, m_backstrides);
        init_tensor(m_shape);
        m_data[0] = T();
    }

    /**
     * Allocates a jltensor with a nested initializer list.
     */
    template <class T, std::size_t N>
    inline jltensor<T, N>::jltensor(nested_initializer_list_t<T, N> t)
        : base_type()
    {
        base_type::resize(xt::shape<shape_type>(t), true);
        nested_copy(this->begin(), t);
    }

    /**
     * Allocates an uninitialized jltensor with the specified shape and
     * layout.
     * @param shape the shape of the jltensor
     */
    template <class T, std::size_t N>
    inline jltensor<T, N>::jltensor(const shape_type& shape)
        : m_shape(shape)
    {
        xt::compute_strides(m_shape, layout_type::column_major, m_strides, m_backstrides);
        init_tensor(m_shape);
    }

    /**
     * Allocates a jltensor with the specified shape and layout. Elements
     * are initialized to the specified value.
     * @param shape the shape of the jltensor
     * @param value the value of the elements
     */
    template <class T, std::size_t N>
    inline jltensor<T, N>::jltensor(const shape_type& shape,
                                    const_reference value)
        : jltensor(shape)
    {
        std::fill(m_data.begin(), m_data.end(), value);
    }

    /**
     * Allocates a jltensor that holds 1 element.
     */
    template <class T, std::size_t N>
    inline jltensor<T, N>::jltensor(jl_array_t* jl)
        : base_type(jl)
    {
        init_from_julia();
    }

    template <class T, std::size_t N>
    template <class S>
    inline jltensor<T, N> jltensor<T, N>::from_shape(S&& shape)
    {
        auto shp = xtl::forward_sequence<shape_type, S>(shape);
        return self_type(shp);
    }
    //@}

    /**
     * @name Copy semantic
     */
    //@{
    /**
     * The copy constructor.
     */
    template <class T, std::size_t N>
    inline jltensor<T, N>::jltensor(const self_type& rhs)
        : base_type(), semantic_base(rhs), m_shape(rhs.shape())
    {
        xt::compute_strides(m_shape, layout_type::column_major, m_strides, m_backstrides);
        init_tensor(m_shape);
        std::copy(rhs.storage().cbegin(), rhs.storage().cend(), this->storage().begin());
    }

    /**
     * The assignment operator.
     */
    template <class T, std::size_t N>
    inline auto jltensor<T, N>::operator=(const self_type& rhs) -> self_type&
    {
        self_type tmp(rhs);
        *this = std::move(tmp);
        return *this;
    }
    //@}

    /**
     * @name Extended copy semantic
     */
    //@{
    /**
     * The extended copy constructor.
     */
    template <class T, std::size_t N>
    template <class E>
    inline jltensor<T, N>::jltensor(const xexpression<E>& e)
        : base_type()
    {
        m_shape = xtl::forward_sequence<shape_type, decltype(e.derived_cast().shape())>(e.derived_cast().shape());
        xt::compute_strides(m_shape, layout_type::column_major, m_strides, m_backstrides);
        init_tensor(m_shape);
        semantic_base::assign(e);
    }

    /**
     * The extended assignment operator.
     */
    template <class T, std::size_t N>
    template <class E>
    inline auto jltensor<T, N>::operator=(const xexpression<E>& e) -> self_type&
    {
        return semantic_base::operator=(e);
    }
    //@}

    template <class T, std::size_t N>
    inline void jltensor<T, N>::init_tensor(const shape_type& shape)
    {
        jl_value_t* array_type;
        jl_datatype_t* tuple_type;
        jl_value_t* dims;

        JL_GC_PUSH3(&array_type, &tuple_type, &dims);

        array_type = make_julia_array_type<value_type>(N);
        tuple_type = make_julia_shape_type(N);

        // allocate array
        dims = jl_new_bits((jl_value_t*)tuple_type, const_cast<void*>(reinterpret_cast<const void*>(shape.data())));
        this->p_array = jl_new_array((jl_value_t*)array_type, dims);

        // setup buffer adaptor
        m_data = storage_type(reinterpret_cast<pointer>(this->p_array->data),
                              static_cast<size_type>(jl_array_len(this->p_array)));
        JL_GC_POP();
    }

    template <class T, std::size_t N>
    inline void jltensor<T, N>::init_from_julia()
    {
        if (this->p_array->flags.ndims != N)
        {
            throw std::runtime_error("Julia array has incorrect number of dimensions");
        }

        std::copy(&(this->p_array->nrows), &(this->p_array->nrows) + N, m_shape.begin());
        xt::compute_strides(m_shape, layout_type::column_major, m_strides, m_backstrides);
        m_data = storage_type(reinterpret_cast<pointer>(this->p_array->data),
                              static_cast<size_type>(jl_array_len(this->p_array)));
    }

    template <class T, std::size_t N>
    inline auto jltensor<T, N>::shape_impl() noexcept -> inner_shape_type&
    {
        return m_shape;
    }

    template <class T, std::size_t N>
    inline auto jltensor<T, N>::shape_impl() const noexcept -> const inner_shape_type&
    {
        return m_shape;
    }

    template <class T, std::size_t N>
    inline auto jltensor<T, N>::strides_impl() noexcept -> inner_strides_type&
    {
        return m_strides;
    }

    template <class T, std::size_t N>
    inline auto jltensor<T, N>::strides_impl() const noexcept -> const inner_strides_type&
    {
        return m_strides;
    }

    template <class T, std::size_t N>
    inline auto jltensor<T, N>::backstrides_impl() noexcept -> inner_backstrides_type&
    {
        return m_backstrides;
    }

    template <class T, std::size_t N>
    inline auto jltensor<T, N>::backstrides_impl() const noexcept -> const inner_backstrides_type&
    {
        return m_backstrides;
    }

    template <class T, std::size_t N>
    inline auto jltensor<T, N>::storage_impl() noexcept -> storage_type&
    {
        return m_data;
    }

    template <class T, std::size_t N>
    inline auto jltensor<T, N>::storage_impl() const noexcept -> const storage_type&
    {
        return m_data;
    }
}

namespace jlcxx
{
    /****************************************************************
     * Template specializations for ConvertToJulia and ConvertToCpp *
     ****************************************************************/

    template <class T, std::size_t N>
    struct ConvertToJulia<xt::jltensor<T, N>>
    {
        template <class U>
        jl_array_t* operator()(U&& arr) const
        {
            return arr.wrapped();
        }
    };

    template <class T, std::size_t N>
    struct ConvertToCpp<xt::jltensor<T, N>>
    {
        xt::jltensor<T, N> operator()(jl_array_t* arr) const
        {
            return xt::jltensor<T, N>(arr);
        }
    };

    template <class T, std::size_t N>
    struct static_type_mapping<xt::jltensor<T, N>>
    {
        using type = jl_array_t*;
    };

    template <class T, std::size_t N>
    struct julia_type_factory<xt::jltensor<T, N>>
    {
        static jl_datatype_t* julia_type()
        {
            create_if_not_exists<T>();
            return (jl_datatype_t*)apply_array_type(jlcxx::julia_type<T>(), N);
        }
    };
}

#endif
