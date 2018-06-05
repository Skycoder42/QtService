#ifndef QTSERVICE_HELPERTYPES_H
#define QTSERVICE_HELPERTYPES_H

#include <functional>
#include <utility>

#include <QtCore/qvariant.h>

namespace QtService {
namespace __helpertypes {

template <typename T>
struct fn_info : public fn_info<decltype(&T::operator())> {};

template <typename TClass, typename TRet, typename... TArgs>
struct fn_info<TRet(TClass::*)(TArgs...) const>
{
	template <typename TFunctor>
	static inline std::function<QVariant(QVariantList)> pack(const TFunctor &fn) {
		return pack(std::make_index_sequence<sizeof...(TArgs)>{}, fn);
	}

	template <typename TFunctor, std::size_t... Is>
	static inline std::function<QVariant(QVariantList)> pack(const std::index_sequence<Is...> &, const TFunctor &fn) {
		return [fn](const QVariantList &args) {
			Q_UNUSED(args)
			return QVariant::fromValue<TRet>(fn(args[Is].template value<std::decay_t<TArgs>>()...));
		};
	}
};

template <typename TClass, typename... TArgs>
struct fn_info<void(TClass::*)(TArgs...) const>
{
	template <typename TFunctor>
	static inline std::function<QVariant(QVariantList)> pack(const TFunctor &fn) {
		return pack(std::make_index_sequence<sizeof...(TArgs)>{}, fn);
	}

	template <typename TFunctor, std::size_t... Is>
	static inline std::function<QVariant(QVariantList)> pack(const std::index_sequence<Is...> &, const TFunctor &fn) {
		return [fn](const QVariantList &args) {
			Q_UNUSED(args)
			fn(args[Is].template value<std::decay_t<TArgs>>()...);
			return QVariant{};
		};
	}
};

template <typename TFunc>
inline std::function<QVariant(QVariantList)> pack_function(const TFunc &fn) {
	return fn_info<TFunc>::pack(fn);
}

}
}
#endif // QTSERVICE_HELPERTYPES_H
