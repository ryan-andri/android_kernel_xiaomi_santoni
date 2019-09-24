#ifndef __NET_NS_HASH_H__
#define __NET_NS_HASH_H__

#include <net/net_namespace.h>

static inline u32 net_hash_mix(const struct net *net)
{
#ifdef CONFIG_NET_NS
	return net->hash_mix;
#else
	return 0;
#endif
}
#endif
