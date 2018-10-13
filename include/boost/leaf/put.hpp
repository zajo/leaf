//Copyright (c) 2018 Emil Dotchevski
//Copyright (c) 2018 Second Spectrum, Inc.

//Distributed under the Boost Software License, Version 1.0. (See accompanying
//file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef UUID_8F1C53BEB39F11E8A1C5B6F3E99C4353
#define UUID_8F1C53BEB39F11E8A1C5B6F3E99C4353

#include <boost/leaf/detail/tl_slot.hpp>
#include <tuple>
#include <type_traits>
#include <exception>

#define ei_SOURCE_LOCATION\
	::boost::leaf::ei_source_location<::boost::leaf::in_function> {__FUNCTION__},\
	::boost::leaf::ei_source_location<::boost::leaf::at_line> {__LINE__},\
	::boost::leaf::ei_source_location<::boost::leaf::in_file> {__FILE__}

namespace
boost
	{
	namespace
	leaf
		{
		template <class... ErrorInfo>
		void
		put( ErrorInfo && ... a ) noexcept
			{
			{ using _ = bool[ ]; (void) _ { leaf_detail::tl_slot<ErrorInfo>::tl_instance().put(std::forward<ErrorInfo>(a))... }; }
			}
		template <class... ErrorInfo,class Exception>
		[[noreturn]]
		void
		throw_exception( Exception const & e, ErrorInfo && ... a )
			{
			leaf_detail::tl_slot_base::bump_current_seq_id();
			put(std::forward<ErrorInfo>(a)...);
			throw e;
			}
		template <class Exception>
		[[noreturn]]
		void
		throw_exception( Exception const & e )
			{
			leaf_detail::tl_slot_base::bump_current_seq_id();
			throw e;
			}
		namespace
		leaf_detail
			{
			template<class F, class E = void> struct can_haz_call { static constexpr bool value=false; };
			template<class F> struct can_haz_call<F, decltype(std::declval<F>()(), void())> { static constexpr bool value=true; };
			template <class T,bool Deferred=can_haz_call<T>::value>
			struct
			defer_dispatch
				{
				static
				void
				put_( T && x ) noexcept
					{
					put(std::move(x));
					}
				};
			template <class F>
			struct
			defer_dispatch<F,true>
				{
				static
				void
				put_( F && x ) noexcept
					{
					put(x());
					}
				};
			template <int I,class Tuple>
			struct
			put_meta
				{
				static
				void
				put( Tuple && t ) noexcept
					{
					typedef typename std::tuple_element<I-1,Tuple>::type ith_type;
					defer_dispatch<ith_type>::put_(std::move(std::get<I-1>(std::move(t))));
					put_meta<I-1,Tuple>::put(std::move(t));
					}
				};
			template <class Tuple>
			struct
			put_meta<0,Tuple>
				{
				static void put( Tuple && ) noexcept { }
				};
			template <class... T>
			class
			preloaded
				{
				preloaded( preloaded const & ) = delete;
				preloaded & operator=( preloaded const & ) = delete;
				typedef std::tuple<typename std::remove_const<typename std::remove_reference<T>::type>::type...> tuple_type;
				optional<tuple_type> to_put_;
				public:
				template <class... U>
				explicit
				preloaded( U && ... a ):
					to_put_(tuple_type(std::forward<U>(a)...))
					{
					}
				preloaded( preloaded && x ) noexcept:
					to_put_(std::move(x.to_put_))
					{
					assert(!x.to_put_.has_value());
					}
				~preloaded() noexcept
					{
					if( to_put_.has_value() && has_current_error() )
						put_meta<sizeof...(T),tuple_type>::put(to_put_.extract_value());
					}
				void
				cancel() noexcept
					{
					to_put_.reset();
					}
				};
			}
		template <class... T>
		leaf_detail::preloaded<T...>
		preload( T && ... a )
			{
			return leaf_detail::preloaded<T...>(std::forward<T>(a)...);
			}
		}
	}

#define LEAF_THROW(e) ::boost::leaf::throw_exception(e,ei_SOURCE_LOCATION)

#endif
