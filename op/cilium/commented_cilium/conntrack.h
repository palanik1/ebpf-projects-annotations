/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/* Copyright Authors of Cilium */

#ifndef __LIB_CONNTRACK_H_
#define __LIB_CONNTRACK_H_

#include <linux/icmpv6.h>
#include <linux/icmp.h>

#include <bpf/verifier.h>

#include "common.h"
#include "utils.h"
#include "ipv4.h"
#include "ipv6.h"
#include "dbg.h"
#include "l4.h"
#include "signal.h"

enum {
	ACTION_UNSPEC,
	ACTION_CREATE,
	ACTION_CLOSE,
};

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 26,
  "endLine": 32,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_entry_seen_both_syns",
  "developer_inline_comments": [
    {
      "start_line": 1,
      "end_line": 1,
      "text": "/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */"
    },
    {
      "start_line": 2,
      "end_line": 2,
      "text": "/* Copyright Authors of Cilium */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "const struct ct_entry *entry"
  ],
  "output": "static__always_inlinebool",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline bool ct_entry_seen_both_syns (const struct ct_entry *entry)\n",
    "{\n",
    "    bool rx_syn = entry->rx_flags_seen & TCP_FLAG_SYN;\n",
    "    bool tx_syn = entry->tx_flags_seen & TCP_FLAG_SYN;\n",
    "    return rx_syn && tx_syn;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline bool ct_entry_seen_both_syns(const struct ct_entry *entry)
{
	bool rx_syn = entry->rx_flags_seen & TCP_FLAG_SYN;
	bool tx_syn = entry->tx_flags_seen & TCP_FLAG_SYN;

	return rx_syn && tx_syn;
}

union tcp_flags {
	struct {
		__u8 upper_bits;
		__u8 lower_bits;
		__u16 pad;
	};
	__u32 value;
};

/**
 * Update the CT timeout and TCP flags for the specified entry.
 *
 * We track the OR'd accumulation of seen tcp flags in the entry, and the
 * last time that a notification was sent. Multiple CPUs may enter this
 * function with packets for the same connection, in which case it is possible
 * for the CPUs to race to update the entry. In such a case, the critical
 * update section may be entered in quick succession, leading to multiple
 * updates of the entry and returning true for each CPU. The BPF architecture
 * guarantees that entire 8-bit or 32-bit values will be set within the entry,
 * so although the CPUs may race, the worst result is that multiple executions
 * of this function return non-zero for the same connection within short
 * succession, leading to multiple trace notifications being sent when one
 * might otherwise expect such notifications to be aggregated.
 *
 * Returns how many bytes of the packet should be monitored:
 * - Zero if this flow was recently monitored.
 * - Non-zero if this flow has not been monitored recently.
 */
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 62,
  "endLine": 126,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "__ct_update_timeout",
  "developer_inline_comments": [
    {
      "start_line": 43,
      "end_line": 61,
      "text": "/**\n * Update the CT timeout and TCP flags for the specified entry.\n *\n * We track the OR'd accumulation of seen tcp flags in the entry, and the\n * last time that a notification was sent. Multiple CPUs may enter this\n * function with packets for the same connection, in which case it is possible\n * for the CPUs to race to update the entry. In such a case, the critical\n * update section may be entered in quick succession, leading to multiple\n * updates of the entry and returning true for each CPU. The BPF architecture\n * guarantees that entire 8-bit or 32-bit values will be set within the entry,\n * so although the CPUs may race, the worst result is that multiple executions\n * of this function return non-zero for the same connection within short\n * succession, leading to multiple trace notifications being sent when one\n * might otherwise expect such notifications to be aggregated.\n *\n * Returns how many bytes of the packet should be monitored:\n * - Zero if this flow was recently monitored.\n * - Non-zero if this flow has not been monitored recently.\n */"
    },
    {
      "start_line": 83,
      "end_line": 112,
      "text": "/* It's possible for multiple CPUs to execute the branch statement here\n\t * one after another, before the first CPU is able to execute the entry\n\t * modifications within this branch. This is somewhat unlikely because\n\t * packets for the same connection are typically steered towards the\n\t * same CPU, but is possible in theory.\n\t *\n\t * If the branch is taken by multiple CPUs because of '*last_report',\n\t * then this merely causes multiple notifications to be sent after\n\t * CT_REPORT_INTERVAL rather than a single notification. '*last_report'\n\t * will be updated by all CPUs and subsequent checks should not take\n\t * this branch until the next CT_REPORT_INTERVAL. As such, the trace\n\t * aggregation that uses the result of this function may reduce the\n\t * number of packets per interval to a small integer value (max N_CPUS)\n\t * rather than 1 notification per packet throughout the interval.\n\t *\n\t * Similar behaviour may happen with tcp_flags. The worst case race\n\t * here would be that two or more CPUs argue over which flags have been\n\t * seen and overwrite each other, with each CPU interleaving different\n\t * values for which flags were seen. In practice, realistic connections\n\t * are likely to progressively set SYN, ACK, then much later perhaps\n\t * FIN and/or RST. Furthermore, unless such a traffic pattern were\n\t * constantly received, this should self-correct as the stored\n\t * tcp_flags is an OR'd set of flags and each time the above code is\n\t * executed, it pulls the latest set of accumulated flags. Therefore\n\t * even in the worst case such a conflict is likely only to cause a\n\t * small number of additional notifications, which is still likely to\n\t * be significantly less under this MONITOR_AGGREGATION mode than would\n\t * otherwise be sent if the MONITOR_AGGREGATION level is set to none\n\t * (ie, sending a notification for every packet).\n\t */"
    },
    {
      "start_line": 115,
      "end_line": 115,
      "text": "/* verifier workaround: we don't use reference here. */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ct_entry *entry",
    " __u32 lifetime",
    " int dir",
    " union tcp_flags flags",
    " __u8 report_mask"
  ],
  "output": "static__always_inline__u32",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline __u32 __ct_update_timeout (struct ct_entry *entry, __u32 lifetime, int dir, union tcp_flags flags, __u8 report_mask)\n",
    "{\n",
    "    __u32 now = bpf_mono_now ();\n",
    "    __u8 accumulated_flags;\n",
    "    __u8 seen_flags = flags.lower_bits & report_mask;\n",
    "    __u32 last_report;\n",
    "    WRITE_ONCE (entry->lifetime, now + lifetime);\n",
    "    if (dir == CT_INGRESS) {\n",
    "        accumulated_flags = READ_ONCE (entry -> rx_flags_seen);\n",
    "        last_report = READ_ONCE (entry -> last_rx_report);\n",
    "    }\n",
    "    else {\n",
    "        accumulated_flags = READ_ONCE (entry -> tx_flags_seen);\n",
    "        last_report = READ_ONCE (entry -> last_tx_report);\n",
    "    }\n",
    "    seen_flags |= accumulated_flags;\n",
    "    if (last_report + bpf_sec_to_mono (CT_REPORT_INTERVAL) < now || accumulated_flags != seen_flags) {\n",
    "        if (dir == CT_INGRESS) {\n",
    "            WRITE_ONCE (entry->rx_flags_seen, seen_flags);\n",
    "            WRITE_ONCE (entry->last_rx_report, now);\n",
    "        }\n",
    "        else {\n",
    "            WRITE_ONCE (entry->tx_flags_seen, seen_flags);\n",
    "            WRITE_ONCE (entry->last_tx_report, now);\n",
    "        }\n",
    "        return TRACE_PAYLOAD_LEN;\n",
    "    }\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "bpf_mono_now",
    "WRITE_ONCE",
    "bpf_sec_to_mono",
    "READ_ONCE"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline __u32 __ct_update_timeout(struct ct_entry *entry,
						 __u32 lifetime, int dir,
						 union tcp_flags flags,
						 __u8 report_mask)
{
	__u32 now = bpf_mono_now();
	__u8 accumulated_flags;
	__u8 seen_flags = flags.lower_bits & report_mask;
	__u32 last_report;

	WRITE_ONCE(entry->lifetime, now + lifetime);

	if (dir == CT_INGRESS) {
		accumulated_flags = READ_ONCE(entry->rx_flags_seen);
		last_report = READ_ONCE(entry->last_rx_report);
	} else {
		accumulated_flags = READ_ONCE(entry->tx_flags_seen);
		last_report = READ_ONCE(entry->last_tx_report);
	}
	seen_flags |= accumulated_flags;

	/* It's possible for multiple CPUs to execute the branch statement here
	 * one after another, before the first CPU is able to execute the entry
	 * modifications within this branch. This is somewhat unlikely because
	 * packets for the same connection are typically steered towards the
	 * same CPU, but is possible in theory.
	 *
	 * If the branch is taken by multiple CPUs because of '*last_report',
	 * then this merely causes multiple notifications to be sent after
	 * CT_REPORT_INTERVAL rather than a single notification. '*last_report'
	 * will be updated by all CPUs and subsequent checks should not take
	 * this branch until the next CT_REPORT_INTERVAL. As such, the trace
	 * aggregation that uses the result of this function may reduce the
	 * number of packets per interval to a small integer value (max N_CPUS)
	 * rather than 1 notification per packet throughout the interval.
	 *
	 * Similar behaviour may happen with tcp_flags. The worst case race
	 * here would be that two or more CPUs argue over which flags have been
	 * seen and overwrite each other, with each CPU interleaving different
	 * values for which flags were seen. In practice, realistic connections
	 * are likely to progressively set SYN, ACK, then much later perhaps
	 * FIN and/or RST. Furthermore, unless such a traffic pattern were
	 * constantly received, this should self-correct as the stored
	 * tcp_flags is an OR'd set of flags and each time the above code is
	 * executed, it pulls the latest set of accumulated flags. Therefore
	 * even in the worst case such a conflict is likely only to cause a
	 * small number of additional notifications, which is still likely to
	 * be significantly less under this MONITOR_AGGREGATION mode than would
	 * otherwise be sent if the MONITOR_AGGREGATION level is set to none
	 * (ie, sending a notification for every packet).
	 */
	if (last_report + bpf_sec_to_mono(CT_REPORT_INTERVAL) < now ||
	    accumulated_flags != seen_flags) {
		/* verifier workaround: we don't use reference here. */
		if (dir == CT_INGRESS) {
			WRITE_ONCE(entry->rx_flags_seen, seen_flags);
			WRITE_ONCE(entry->last_rx_report, now);
		} else {
			WRITE_ONCE(entry->tx_flags_seen, seen_flags);
			WRITE_ONCE(entry->last_tx_report, now);
		}
		return TRACE_PAYLOAD_LEN;
	}
	return 0;
}

/**
 * Update the CT timeouts for the specified entry.
 *
 * If CT_REPORT_INTERVAL has elapsed since the last update, updates the
 * last_updated timestamp and returns true. Otherwise returns false.
 */
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 134,
  "endLine": 156,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_update_timeout",
  "developer_inline_comments": [
    {
      "start_line": 128,
      "end_line": 133,
      "text": "/**\n * Update the CT timeouts for the specified entry.\n *\n * If CT_REPORT_INTERVAL has elapsed since the last update, updates the\n * last_updated timestamp and returns true. Otherwise returns false.\n */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ct_entry *entry",
    " bool tcp",
    " int dir",
    " union tcp_flags seen_flags"
  ],
  "output": "static__always_inline__u32",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline __u32 ct_update_timeout (struct ct_entry *entry, bool tcp, int dir, union tcp_flags seen_flags)\n",
    "{\n",
    "    __u32 lifetime = dir == CT_SERVICE ? bpf_sec_to_mono (CT_SERVICE_LIFETIME_NONTCP) : bpf_sec_to_mono (CT_CONNECTION_LIFETIME_NONTCP);\n",
    "    bool syn = seen_flags.value & TCP_FLAG_SYN;\n",
    "    if (tcp) {\n",
    "        entry->seen_non_syn |= !syn;\n",
    "        if (entry->seen_non_syn) {\n",
    "            lifetime = dir == CT_SERVICE ? bpf_sec_to_mono (CT_SERVICE_LIFETIME_TCP) : bpf_sec_to_mono (CT_CONNECTION_LIFETIME_TCP);\n",
    "        }\n",
    "        else {\n",
    "            lifetime = bpf_sec_to_mono (CT_SYN_TIMEOUT);\n",
    "        }\n",
    "    }\n",
    "    return __ct_update_timeout (entry, lifetime, dir, seen_flags, CT_REPORT_FLAGS);\n",
    "}\n"
  ],
  "called_function_list": [
    "__ct_update_timeout",
    "bpf_sec_to_mono"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline __u32 ct_update_timeout(struct ct_entry *entry,
					       bool tcp, int dir,
					       union tcp_flags seen_flags)
{
	__u32 lifetime = dir == CT_SERVICE ?
			 bpf_sec_to_mono(CT_SERVICE_LIFETIME_NONTCP) :
			 bpf_sec_to_mono(CT_CONNECTION_LIFETIME_NONTCP);
	bool syn = seen_flags.value & TCP_FLAG_SYN;

	if (tcp) {
		entry->seen_non_syn |= !syn;
		if (entry->seen_non_syn) {
			lifetime = dir == CT_SERVICE ?
				   bpf_sec_to_mono(CT_SERVICE_LIFETIME_TCP) :
				   bpf_sec_to_mono(CT_CONNECTION_LIFETIME_TCP);
		} else {
			lifetime = bpf_sec_to_mono(CT_SYN_TIMEOUT);
		}
	}

	return __ct_update_timeout(entry, lifetime, dir, seen_flags,
				   CT_REPORT_FLAGS);
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 158,
  "endLine": 162,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_reset_closing",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ct_entry *entry"
  ],
  "output": "static__always_inlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_reset_closing (struct ct_entry *entry)\n",
    "{\n",
    "    entry->rx_closing = 0;\n",
    "    entry->tx_closing = 0;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void ct_reset_closing(struct ct_entry *entry)
{
	entry->rx_closing = 0;
	entry->tx_closing = 0;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 164,
  "endLine": 167,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_entry_alive",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "const struct ct_entry *entry"
  ],
  "output": "static__always_inlinebool",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline bool ct_entry_alive (const struct ct_entry *entry)\n",
    "{\n",
    "    return !entry->rx_closing || !entry->tx_closing;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline bool ct_entry_alive(const struct ct_entry *entry)
{
	return !entry->rx_closing || !entry->tx_closing;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 169,
  "endLine": 172,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_entry_closing",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "const struct ct_entry *entry"
  ],
  "output": "static__always_inlinebool",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline bool ct_entry_closing (const struct ct_entry *entry)\n",
    "{\n",
    "    return entry->tx_closing || entry->rx_closing;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline bool ct_entry_closing(const struct ct_entry *entry)
{
	return entry->tx_closing || entry->rx_closing;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 174,
  "endLine": 183,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_entry_expired_rebalance",
  "developer_inline_comments": [
    {
      "start_line": 179,
      "end_line": 181,
      "text": "/* This doesn't check last_rx_report because we don't see closing\n\t * in RX direction for CT_SERVICE.\n\t */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "const struct ct_entry *entry"
  ],
  "output": "static__always_inlinebool",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline bool ct_entry_expired_rebalance (const struct ct_entry *entry)\n",
    "{\n",
    "    __u32 wait_time = bpf_sec_to_mono (CT_SERVICE_CLOSE_REBALANCE);\n",
    "    return READ_ONCE (entry->last_tx_report) + wait_time <= bpf_mono_now ();\n",
    "}\n"
  ],
  "called_function_list": [
    "bpf_mono_now",
    "bpf_sec_to_mono",
    "READ_ONCE"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline bool
ct_entry_expired_rebalance(const struct ct_entry *entry)
{
	__u32 wait_time = bpf_sec_to_mono(CT_SERVICE_CLOSE_REBALANCE);

	/* This doesn't check last_rx_report because we don't see closing
	 * in RX direction for CT_SERVICE.
	 */
	return READ_ONCE(entry->last_tx_report) + wait_time <= bpf_mono_now();
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 185,
  "endLine": 273,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "__ct_lookup",
  "developer_inline_comments": [
    {
      "start_line": 222,
      "end_line": 222,
      "text": "/* FIXME: This is slow, per-cpu counters? */"
    },
    {
      "start_line": 243,
      "end_line": 247,
      "text": "/* If we got an RST and have not seen both SYNs,\n\t\t\t * terminate the connection. (For CT_SERVICE, we do not\n\t\t\t * see both directions, so flags of established\n\t\t\t * connections would not include both SYNs.)\n\t\t\t */"
    }
  ],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " struct  __ctx_buff *ctx",
    " const void *tuple",
    " int action",
    " int dir",
    " struct ct_state *ct_state",
    " bool is_tcp",
    " union tcp_flags seen_flags",
    " __u32 *monitor"
  ],
  "output": "static__always_inline__u8",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline __u8 __ct_lookup (const void *map, struct  __ctx_buff *ctx, const void *tuple, int action, int dir, struct ct_state *ct_state, bool is_tcp, union tcp_flags seen_flags, __u32 *monitor)\n",
    "{\n",
    "    bool syn = seen_flags.value & TCP_FLAG_SYN;\n",
    "    struct ct_entry *entry;\n",
    "    int reopen;\n",
    "    relax_verifier ();\n",
    "    entry = map_lookup_elem (map, tuple);\n",
    "    if (entry) {\n",
    "        cilium_dbg (ctx, DBG_CT_MATCH, entry->lifetime, entry->rev_nat_index);\n",
    "\n",
    "#ifdef HAVE_LARGE_INSN_LIMIT\n",
    "        if (dir == CT_SERVICE && syn && ct_entry_closing (entry) && ct_entry_expired_rebalance (entry))\n",
    "            goto ct_new;\n",
    "\n",
    "#endif\n",
    "        if (ct_entry_alive (entry))\n",
    "            *monitor = ct_update_timeout (entry, is_tcp, dir, seen_flags);\n",
    "        if (ct_state) {\n",
    "            ct_state->rev_nat_index = entry->rev_nat_index;\n",
    "            ct_state->loopback = entry->lb_loopback;\n",
    "            ct_state->node_port = entry->node_port;\n",
    "            ct_state->ifindex = entry->ifindex;\n",
    "            ct_state->dsr = entry->dsr;\n",
    "            ct_state->proxy_redirect = entry->proxy_redirect;\n",
    "            ct_state->from_l7lb = entry->from_l7lb;\n",
    "            if (dir == CT_SERVICE) {\n",
    "                ct_state->backend_id = entry->backend_id;\n",
    "                ct_state->syn = syn;\n",
    "            }\n",
    "        }\n",
    "\n",
    "#ifdef CONNTRACK_ACCOUNTING\n",
    "        if (dir == CT_INGRESS) {\n",
    "            __sync_fetch_and_add (&entry->rx_packets, 1);\n",
    "            __sync_fetch_and_add (&entry->rx_bytes, ctx_full_len (ctx));\n",
    "        }\n",
    "        else if (dir == CT_EGRESS) {\n",
    "            __sync_fetch_and_add (&entry->tx_packets, 1);\n",
    "            __sync_fetch_and_add (&entry->tx_bytes, ctx_full_len (ctx));\n",
    "        }\n",
    "\n",
    "#endif\n",
    "        switch (action) {\n",
    "        case ACTION_CREATE :\n",
    "            reopen = entry->rx_closing | entry->tx_closing;\n",
    "            reopen |= seen_flags.value & TCP_FLAG_SYN;\n",
    "            if (unlikely (reopen == (TCP_FLAG_SYN | 0x1))) {\n",
    "                ct_reset_closing (entry);\n",
    "                *monitor = ct_update_timeout (entry, is_tcp, dir, seen_flags);\n",
    "                return CT_REOPENED;\n",
    "            }\n",
    "            break;\n",
    "        case ACTION_CLOSE :\n",
    "            if (!ct_entry_seen_both_syns (entry) && (seen_flags.value & TCP_FLAG_RST) && dir != CT_SERVICE) {\n",
    "                entry->rx_closing = 1;\n",
    "                entry->tx_closing = 1;\n",
    "            }\n",
    "            else if (dir == CT_INGRESS) {\n",
    "                entry->rx_closing = 1;\n",
    "            }\n",
    "            else {\n",
    "                entry->tx_closing = 1;\n",
    "            }\n",
    "            *monitor = TRACE_PAYLOAD_LEN;\n",
    "            if (ct_entry_alive (entry))\n",
    "                break;\n",
    "            __ct_update_timeout (entry, bpf_sec_to_mono (CT_CLOSE_TIMEOUT), dir, seen_flags, CT_REPORT_FLAGS);\n",
    "            break;\n",
    "        }\n",
    "        return CT_ESTABLISHED;\n",
    "    }\n",
    "ct_new :\n",
    "    __maybe_unused * monitor = TRACE_PAYLOAD_LEN;\n",
    "    return CT_NEW;\n",
    "}\n"
  ],
  "called_function_list": [
    "cilium_dbg",
    "unlikely",
    "__sync_fetch_and_add",
    "ct_entry_seen_both_syns",
    "bpf_sec_to_mono",
    "ctx_full_len",
    "ct_reset_closing",
    "__ct_update_timeout",
    "ct_entry_expired_rebalance",
    "ct_entry_closing",
    "ct_update_timeout",
    "ct_entry_alive",
    "relax_verifier"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline __u8 __ct_lookup(const void *map, struct __ctx_buff *ctx,
					const void *tuple, int action, int dir,
					struct ct_state *ct_state,
					bool is_tcp, union tcp_flags seen_flags,
					__u32 *monitor)
{
	bool syn = seen_flags.value & TCP_FLAG_SYN;
	struct ct_entry *entry;
	int reopen;

	relax_verifier();

	entry = map_lookup_elem(map, tuple);
	if (entry) {
		cilium_dbg(ctx, DBG_CT_MATCH, entry->lifetime, entry->rev_nat_index);
#ifdef HAVE_LARGE_INSN_LIMIT
		if (dir == CT_SERVICE && syn &&
		    ct_entry_closing(entry) &&
		    ct_entry_expired_rebalance(entry))
			goto ct_new;
#endif
		if (ct_entry_alive(entry))
			*monitor = ct_update_timeout(entry, is_tcp, dir, seen_flags);
		if (ct_state) {
			ct_state->rev_nat_index = entry->rev_nat_index;
			ct_state->loopback = entry->lb_loopback;
			ct_state->node_port = entry->node_port;
			ct_state->ifindex = entry->ifindex;
			ct_state->dsr = entry->dsr;
			ct_state->proxy_redirect = entry->proxy_redirect;
			ct_state->from_l7lb = entry->from_l7lb;
			if (dir == CT_SERVICE) {
				ct_state->backend_id = entry->backend_id;
				ct_state->syn = syn;
			}
		}
#ifdef CONNTRACK_ACCOUNTING
		/* FIXME: This is slow, per-cpu counters? */
		if (dir == CT_INGRESS) {
			__sync_fetch_and_add(&entry->rx_packets, 1);
			__sync_fetch_and_add(&entry->rx_bytes, ctx_full_len(ctx));
		} else if (dir == CT_EGRESS) {
			__sync_fetch_and_add(&entry->tx_packets, 1);
			__sync_fetch_and_add(&entry->tx_bytes, ctx_full_len(ctx));
		}
#endif
		switch (action) {
		case ACTION_CREATE:
			reopen = entry->rx_closing | entry->tx_closing;
			reopen |= seen_flags.value & TCP_FLAG_SYN;
			if (unlikely(reopen == (TCP_FLAG_SYN|0x1))) {
				ct_reset_closing(entry);
				*monitor = ct_update_timeout(entry, is_tcp, dir, seen_flags);
				return CT_REOPENED;
			}
			break;

		case ACTION_CLOSE:
			/* If we got an RST and have not seen both SYNs,
			 * terminate the connection. (For CT_SERVICE, we do not
			 * see both directions, so flags of established
			 * connections would not include both SYNs.)
			 */
			if (!ct_entry_seen_both_syns(entry) &&
			    (seen_flags.value & TCP_FLAG_RST) &&
			    dir != CT_SERVICE) {
				entry->rx_closing = 1;
				entry->tx_closing = 1;
			} else if (dir == CT_INGRESS) {
				entry->rx_closing = 1;
			} else {
				entry->tx_closing = 1;
			}

			*monitor = TRACE_PAYLOAD_LEN;
			if (ct_entry_alive(entry))
				break;
			__ct_update_timeout(entry, bpf_sec_to_mono(CT_CLOSE_TIMEOUT),
					    dir, seen_flags, CT_REPORT_FLAGS);
			break;
		}

		return CT_ESTABLISHED;
	}

ct_new: __maybe_unused
	*monitor = TRACE_PAYLOAD_LEN;
	return CT_NEW;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "pkt_go_to_next_module",
      "pkt_go_to_next_module": [
        {
          "Project": "cilium",
          "Return Type": "int",
          "Input Params": [],
          "Function Name": "TC_ACT_OK",
          "Return": 0,
          "Description": "will terminate the packet processing pipeline and allows the packet to proceed. Pass the skb onwards either to upper layers of the stack on ingress or down to the networking device driver for transmission on egress, respectively. TC_ACT_OK sets skb->tc_index based on the classid the tc BPF program set. The latter is set out of the tc BPF program itself through skb->tc_classid from the BPF context.",
          "compatible_hookpoints": [
            "xdp",
            "sched_cls",
            "sched_act"
          ],
          "capabilities": [
            "pkt_go_to_next_module"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 275,
  "endLine": 303,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ipv6_extract_tuple",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx",
    " struct ipv6_ct_tuple *tuple",
    " int *l4_off"
  ],
  "output": "static__always_inlineint",
  "helper": [
    "CTX_ACT_OK"
  ],
  "compatibleHookpoints": [
    "xdp",
    "sched_act",
    "sched_cls"
  ],
  "source": [
    "static __always_inline int ipv6_extract_tuple (struct  __ctx_buff *ctx, struct ipv6_ct_tuple *tuple, int *l4_off)\n",
    "{\n",
    "    int ret, l3_off = ETH_HLEN;\n",
    "    void *data, *data_end;\n",
    "    struct ipv6hdr *ip6;\n",
    "    if (!revalidate_data (ctx, &data, &data_end, &ip6))\n",
    "        return DROP_INVALID;\n",
    "    tuple->nexthdr = ip6->nexthdr;\n",
    "    ipv6_addr_copy (&tuple->daddr, (union v6addr *) &ip6->daddr);\n",
    "    ipv6_addr_copy (&tuple->saddr, (union v6addr *) &ip6->saddr);\n",
    "    ret = ipv6_hdrlen (ctx, & tuple -> nexthdr);\n",
    "    if (ret < 0)\n",
    "        return ret;\n",
    "    if (unlikely (tuple->nexthdr != IPPROTO_TCP && tuple->nexthdr != IPPROTO_UDP))\n",
    "        return DROP_CT_UNKNOWN_PROTO;\n",
    "    if (ret < 0)\n",
    "        return ret;\n",
    "    *l4_off = l3_off + ret;\n",
    "    return CTX_ACT_OK;\n",
    "}\n"
  ],
  "called_function_list": [
    "revalidate_data",
    "ipv6_addr_copy",
    "unlikely",
    "ipv6_hdrlen"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int
ipv6_extract_tuple(struct __ctx_buff *ctx, struct ipv6_ct_tuple *tuple,
		   int *l4_off)
{
	int ret, l3_off = ETH_HLEN;
	void *data, *data_end;
	struct ipv6hdr *ip6;

	if (!revalidate_data(ctx, &data, &data_end, &ip6))
		return DROP_INVALID;

	tuple->nexthdr = ip6->nexthdr;
	ipv6_addr_copy(&tuple->daddr, (union v6addr *)&ip6->daddr);
	ipv6_addr_copy(&tuple->saddr, (union v6addr *)&ip6->saddr);

	ret = ipv6_hdrlen(ctx, &tuple->nexthdr);
	if (ret < 0)
		return ret;

	if (unlikely(tuple->nexthdr != IPPROTO_TCP &&
		     tuple->nexthdr != IPPROTO_UDP))
		return DROP_CT_UNKNOWN_PROTO;

	if (ret < 0)
		return ret;

	*l4_off = l3_off + ret;
	return CTX_ACT_OK;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 305,
  "endLine": 311,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_flip_tuple_dir6",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv6_ct_tuple *tuple"
  ],
  "output": "static__always_inlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_flip_tuple_dir6 (struct ipv6_ct_tuple *tuple)\n",
    "{\n",
    "    if (tuple->flags & TUPLE_F_IN)\n",
    "        tuple->flags &= ~TUPLE_F_IN;\n",
    "    else\n",
    "        tuple->flags |= TUPLE_F_IN;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void ct_flip_tuple_dir6(struct ipv6_ct_tuple *tuple)
{
	if (tuple->flags & TUPLE_F_IN)
		tuple->flags &= ~TUPLE_F_IN;
	else
		tuple->flags |= TUPLE_F_IN;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 313,
  "endLine": 326,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "__ipv6_ct_tuple_reverse",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv6_ct_tuple *tuple"
  ],
  "output": "static__always_inlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void __ipv6_ct_tuple_reverse (struct ipv6_ct_tuple *tuple)\n",
    "{\n",
    "    union v6addr tmp_addr = {}\n",
    "    ;\n",
    "    __be16 tmp;\n",
    "    ipv6_addr_copy (&tmp_addr, &tuple->saddr);\n",
    "    ipv6_addr_copy (&tuple->saddr, &tuple->daddr);\n",
    "    ipv6_addr_copy (&tuple->daddr, &tmp_addr);\n",
    "    tmp = tuple->sport;\n",
    "    tuple->sport = tuple->dport;\n",
    "    tuple->dport = tmp;\n",
    "}\n"
  ],
  "called_function_list": [
    "ipv6_addr_copy"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
__ipv6_ct_tuple_reverse(struct ipv6_ct_tuple *tuple)
{
	union v6addr tmp_addr = {};
	__be16 tmp;

	ipv6_addr_copy(&tmp_addr, &tuple->saddr);
	ipv6_addr_copy(&tuple->saddr, &tuple->daddr);
	ipv6_addr_copy(&tuple->daddr, &tmp_addr);

	tmp = tuple->sport;
	tuple->sport = tuple->dport;
	tuple->dport = tmp;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 328,
  "endLine": 333,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ipv6_ct_tuple_reverse",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv6_ct_tuple *tuple"
  ],
  "output": "static__always_inlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ipv6_ct_tuple_reverse (struct ipv6_ct_tuple *tuple)\n",
    "{\n",
    "    __ipv6_ct_tuple_reverse (tuple);\n",
    "    ct_flip_tuple_dir6 (tuple);\n",
    "}\n"
  ],
  "called_function_list": [
    "__ipv6_ct_tuple_reverse",
    "ct_flip_tuple_dir6"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
ipv6_ct_tuple_reverse(struct ipv6_ct_tuple *tuple)
{
	__ipv6_ct_tuple_reverse(tuple);
	ct_flip_tuple_dir6(tuple);
}

/* Offset must point to IPv6 */
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 336,
  "endLine": 459,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_lookup6",
  "developer_inline_comments": [
    {
      "start_line": 335,
      "end_line": 335,
      "text": "/* Offset must point to IPv6 */"
    },
    {
      "start_line": 346,
      "end_line": 352,
      "text": "/* The tuple is created in reverse order initially to find a\n\t * potential reverse flow. This is required because the RELATED\n\t * or REPLY state takes precedence over ESTABLISHED due to\n\t * policy requirements.\n\t *\n\t * tuple->flags separates entries that could otherwise be overlapping.\n\t */"
    },
    {
      "start_line": 393,
      "end_line": 393,
      "text": "/* fall through */"
    },
    {
      "start_line": 412,
      "end_line": 412,
      "text": "/* load sport + dport into tuple */"
    },
    {
      "start_line": 418,
      "end_line": 418,
      "text": "/* load sport + dport into tuple */"
    },
    {
      "start_line": 426,
      "end_line": 426,
      "text": "/* Can't handle extension headers yet */"
    },
    {
      "start_line": 430,
      "end_line": 434,
      "text": "/* Lookup the reverse direction\n\t *\n\t * This will find an existing flow in the reverse direction.\n\t * The reverse direction is the one where reverse nat index is stored.\n\t */"
    },
    {
      "start_line": 450,
      "end_line": 450,
      "text": "/* Lookup entry in forward direction */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "const void *map",
    " struct ipv6_ct_tuple *tuple",
    " struct  __ctx_buff *ctx",
    " int l4_off",
    " int dir",
    " struct ct_state *ct_state",
    " __u32 *monitor"
  ],
  "output": "static__always_inlineint",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline int ct_lookup6 (const void *map, struct ipv6_ct_tuple *tuple, struct  __ctx_buff *ctx, int l4_off, int dir, struct ct_state *ct_state, __u32 *monitor)\n",
    "{\n",
    "    int ret = CT_NEW, action = ACTION_UNSPEC;\n",
    "    bool is_tcp = tuple->nexthdr == IPPROTO_TCP;\n",
    "    union tcp_flags tcp_flags = {\n",
    "        .value = 0}\n",
    "    ;\n",
    "    if (dir == CT_INGRESS)\n",
    "        tuple->flags = TUPLE_F_OUT;\n",
    "    else if (dir == CT_EGRESS)\n",
    "        tuple->flags = TUPLE_F_IN;\n",
    "    else if (dir == CT_SERVICE)\n",
    "        tuple->flags = TUPLE_F_SERVICE;\n",
    "    else\n",
    "        return DROP_CT_INVALID_HDR;\n",
    "    switch (tuple->nexthdr) {\n",
    "    case IPPROTO_ICMPV6 :\n",
    "        if (1) {\n",
    "            __be16 identifier = 0;\n",
    "            __u8 type;\n",
    "            if (ctx_load_bytes (ctx, l4_off, &type, 1) < 0)\n",
    "                return DROP_CT_INVALID_HDR;\n",
    "            if ((type == ICMPV6_ECHO_REQUEST || type == ICMPV6_ECHO_REPLY) && ctx_load_bytes (ctx, l4_off + offsetof (struct icmp6hdr, icmp6_dataun.u_echo.identifier), &identifier, 2) < 0)\n",
    "                return DROP_CT_INVALID_HDR;\n",
    "            tuple->sport = 0;\n",
    "            tuple->dport = 0;\n",
    "            switch (type) {\n",
    "            case ICMPV6_DEST_UNREACH :\n",
    "            case ICMPV6_PKT_TOOBIG :\n",
    "            case ICMPV6_TIME_EXCEED :\n",
    "            case ICMPV6_PARAMPROB :\n",
    "                tuple->flags |= TUPLE_F_RELATED;\n",
    "                break;\n",
    "            case ICMPV6_ECHO_REPLY :\n",
    "                tuple->sport = identifier;\n",
    "                break;\n",
    "            case ICMPV6_ECHO_REQUEST :\n",
    "                tuple->dport = identifier;\n",
    "            default :\n",
    "                action = ACTION_CREATE;\n",
    "                break;\n",
    "            }\n",
    "        }\n",
    "        break;\n",
    "    case IPPROTO_TCP :\n",
    "        if (1) {\n",
    "            if (ctx_load_bytes (ctx, l4_off + 12, &tcp_flags, 2) < 0)\n",
    "                return DROP_CT_INVALID_HDR;\n",
    "            if (unlikely (tcp_flags.value & (TCP_FLAG_RST | TCP_FLAG_FIN)))\n",
    "                action = ACTION_CLOSE;\n",
    "            else\n",
    "                action = ACTION_CREATE;\n",
    "        }\n",
    "        if (ctx_load_bytes (ctx, l4_off, &tuple->dport, 4) < 0)\n",
    "            return DROP_CT_INVALID_HDR;\n",
    "        break;\n",
    "    case IPPROTO_UDP :\n",
    "        if (ctx_load_bytes (ctx, l4_off, &tuple->dport, 4) < 0)\n",
    "            return DROP_CT_INVALID_HDR;\n",
    "        action = ACTION_CREATE;\n",
    "        break;\n",
    "    default :\n",
    "        return DROP_CT_UNKNOWN_PROTO;\n",
    "    }\n",
    "    cilium_dbg3 (ctx, DBG_CT_LOOKUP6_1, (__u32) tuple->saddr.p4, (__u32) tuple->daddr.p4, (bpf_ntohs (tuple->sport) << 16) | bpf_ntohs (tuple->dport));\n",
    "    cilium_dbg3 (ctx, DBG_CT_LOOKUP6_2, (tuple->nexthdr << 8) | tuple->flags, 0, 0);\n",
    "    ret = __ct_lookup (map, ctx, tuple, action, dir, ct_state, is_tcp, tcp_flags, monitor);\n",
    "    if (ret != CT_NEW) {\n",
    "        if (likely (ret == CT_ESTABLISHED || ret == CT_REOPENED)) {\n",
    "            if (unlikely (tuple->flags & TUPLE_F_RELATED))\n",
    "                ret = CT_RELATED;\n",
    "            else\n",
    "                ret = CT_REPLY;\n",
    "        }\n",
    "        goto out;\n",
    "    }\n",
    "    if (dir != CT_SERVICE) {\n",
    "        ipv6_ct_tuple_reverse (tuple);\n",
    "        ret = __ct_lookup (map, ctx, tuple, action, dir, ct_state, is_tcp, tcp_flags, monitor);\n",
    "    }\n",
    "out :\n",
    "    cilium_dbg (ctx, DBG_CT_VERDICT, ret < 0 ? -ret : ret, ct_state->rev_nat_index);\n",
    "    return ret;\n",
    "}\n"
  ],
  "called_function_list": [
    "cilium_dbg",
    "unlikely",
    "likely",
    "ipv6_ct_tuple_reverse",
    "ctx_load_bytes",
    "offsetof",
    "__ct_lookup",
    "cilium_dbg3",
    "bpf_ntohs"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int ct_lookup6(const void *map,
				      struct ipv6_ct_tuple *tuple,
				      struct __ctx_buff *ctx, int l4_off,
				      int dir, struct ct_state *ct_state,
				      __u32 *monitor)
{
	int ret = CT_NEW, action = ACTION_UNSPEC;
	bool is_tcp = tuple->nexthdr == IPPROTO_TCP;
	union tcp_flags tcp_flags = { .value = 0 };

	/* The tuple is created in reverse order initially to find a
	 * potential reverse flow. This is required because the RELATED
	 * or REPLY state takes precedence over ESTABLISHED due to
	 * policy requirements.
	 *
	 * tuple->flags separates entries that could otherwise be overlapping.
	 */
	if (dir == CT_INGRESS)
		tuple->flags = TUPLE_F_OUT;
	else if (dir == CT_EGRESS)
		tuple->flags = TUPLE_F_IN;
	else if (dir == CT_SERVICE)
		tuple->flags = TUPLE_F_SERVICE;
	else
		return DROP_CT_INVALID_HDR;

	switch (tuple->nexthdr) {
	case IPPROTO_ICMPV6:
		if (1) {
			__be16 identifier = 0;
			__u8 type;

			if (ctx_load_bytes(ctx, l4_off, &type, 1) < 0)
				return DROP_CT_INVALID_HDR;
			if ((type == ICMPV6_ECHO_REQUEST || type == ICMPV6_ECHO_REPLY) &&
			     ctx_load_bytes(ctx, l4_off + offsetof(struct icmp6hdr,
								   icmp6_dataun.u_echo.identifier),
					    &identifier, 2) < 0)
				return DROP_CT_INVALID_HDR;

			tuple->sport = 0;
			tuple->dport = 0;

			switch (type) {
			case ICMPV6_DEST_UNREACH:
			case ICMPV6_PKT_TOOBIG:
			case ICMPV6_TIME_EXCEED:
			case ICMPV6_PARAMPROB:
				tuple->flags |= TUPLE_F_RELATED;
				break;

			case ICMPV6_ECHO_REPLY:
				tuple->sport = identifier;
				break;

			case ICMPV6_ECHO_REQUEST:
				tuple->dport = identifier;
				/* fall through */
			default:
				action = ACTION_CREATE;
				break;
			}
		}
		break;

	case IPPROTO_TCP:
		if (1) {
			if (ctx_load_bytes(ctx, l4_off + 12, &tcp_flags, 2) < 0)
				return DROP_CT_INVALID_HDR;

			if (unlikely(tcp_flags.value & (TCP_FLAG_RST|TCP_FLAG_FIN)))
				action = ACTION_CLOSE;
			else
				action = ACTION_CREATE;
		}

		/* load sport + dport into tuple */
		if (ctx_load_bytes(ctx, l4_off, &tuple->dport, 4) < 0)
			return DROP_CT_INVALID_HDR;
		break;

	case IPPROTO_UDP:
		/* load sport + dport into tuple */
		if (ctx_load_bytes(ctx, l4_off, &tuple->dport, 4) < 0)
			return DROP_CT_INVALID_HDR;

		action = ACTION_CREATE;
		break;

	default:
		/* Can't handle extension headers yet */
		return DROP_CT_UNKNOWN_PROTO;
	}

	/* Lookup the reverse direction
	 *
	 * This will find an existing flow in the reverse direction.
	 * The reverse direction is the one where reverse nat index is stored.
	 */
	cilium_dbg3(ctx, DBG_CT_LOOKUP6_1, (__u32) tuple->saddr.p4, (__u32) tuple->daddr.p4,
		      (bpf_ntohs(tuple->sport) << 16) | bpf_ntohs(tuple->dport));
	cilium_dbg3(ctx, DBG_CT_LOOKUP6_2, (tuple->nexthdr << 8) | tuple->flags, 0, 0);
	ret = __ct_lookup(map, ctx, tuple, action, dir, ct_state, is_tcp,
			  tcp_flags, monitor);
	if (ret != CT_NEW) {
		if (likely(ret == CT_ESTABLISHED || ret == CT_REOPENED)) {
			if (unlikely(tuple->flags & TUPLE_F_RELATED))
				ret = CT_RELATED;
			else
				ret = CT_REPLY;
		}
		goto out;
	}

	/* Lookup entry in forward direction */
	if (dir != CT_SERVICE) {
		ipv6_ct_tuple_reverse(tuple);
		ret = __ct_lookup(map, ctx, tuple, action, dir, ct_state,
				  is_tcp, tcp_flags, monitor);
	}
out:
	cilium_dbg(ctx, DBG_CT_VERDICT, ret < 0 ? -ret : ret, ct_state->rev_nat_index);
	return ret;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "pkt_go_to_next_module",
      "pkt_go_to_next_module": [
        {
          "Project": "cilium",
          "Return Type": "int",
          "Input Params": [],
          "Function Name": "TC_ACT_OK",
          "Return": 0,
          "Description": "will terminate the packet processing pipeline and allows the packet to proceed. Pass the skb onwards either to upper layers of the stack on ingress or down to the networking device driver for transmission on egress, respectively. TC_ACT_OK sets skb->tc_index based on the classid the tc BPF program set. The latter is set out of the tc BPF program itself through skb->tc_classid from the BPF context.",
          "compatible_hookpoints": [
            "xdp",
            "sched_cls",
            "sched_act"
          ],
          "capabilities": [
            "pkt_go_to_next_module"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 461,
  "endLine": 483,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ipv4_extract_tuple",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx",
    " struct ipv4_ct_tuple *tuple",
    " int *l4_off"
  ],
  "output": "static__always_inlineint",
  "helper": [
    "CTX_ACT_OK"
  ],
  "compatibleHookpoints": [
    "xdp",
    "sched_act",
    "sched_cls"
  ],
  "source": [
    "static __always_inline int ipv4_extract_tuple (struct  __ctx_buff *ctx, struct ipv4_ct_tuple *tuple, int *l4_off)\n",
    "{\n",
    "    int l3_off = ETH_HLEN;\n",
    "    void *data, *data_end;\n",
    "    struct iphdr *ip4;\n",
    "    if (!revalidate_data (ctx, &data, &data_end, &ip4))\n",
    "        return DROP_INVALID;\n",
    "    tuple->nexthdr = ip4->protocol;\n",
    "    if (unlikely (tuple->nexthdr != IPPROTO_TCP && tuple->nexthdr != IPPROTO_UDP))\n",
    "        return DROP_CT_UNKNOWN_PROTO;\n",
    "    tuple->daddr = ip4->daddr;\n",
    "    tuple->saddr = ip4->saddr;\n",
    "    *l4_off = l3_off + ipv4_hdrlen (ip4);\n",
    "    return CTX_ACT_OK;\n",
    "}\n"
  ],
  "called_function_list": [
    "revalidate_data",
    "ipv4_hdrlen",
    "unlikely"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int
ipv4_extract_tuple(struct __ctx_buff *ctx, struct ipv4_ct_tuple *tuple,
		   int *l4_off)
{
	int l3_off = ETH_HLEN;
	void *data, *data_end;
	struct iphdr *ip4;

	if (!revalidate_data(ctx, &data, &data_end, &ip4))
		return DROP_INVALID;

	tuple->nexthdr = ip4->protocol;

	if (unlikely(tuple->nexthdr != IPPROTO_TCP &&
		     tuple->nexthdr != IPPROTO_UDP))
		return DROP_CT_UNKNOWN_PROTO;

	tuple->daddr = ip4->daddr;
	tuple->saddr = ip4->saddr;

	*l4_off = l3_off + ipv4_hdrlen(ip4);
	return CTX_ACT_OK;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 485,
  "endLine": 491,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_flip_tuple_dir4",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv4_ct_tuple *tuple"
  ],
  "output": "static__always_inlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_flip_tuple_dir4 (struct ipv4_ct_tuple *tuple)\n",
    "{\n",
    "    if (tuple->flags & TUPLE_F_IN)\n",
    "        tuple->flags &= ~TUPLE_F_IN;\n",
    "    else\n",
    "        tuple->flags |= TUPLE_F_IN;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void ct_flip_tuple_dir4(struct ipv4_ct_tuple *tuple)
{
	if (tuple->flags & TUPLE_F_IN)
		tuple->flags &= ~TUPLE_F_IN;
	else
		tuple->flags |= TUPLE_F_IN;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 493,
  "endLine": 505,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "__ipv4_ct_tuple_reverse",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv4_ct_tuple *tuple"
  ],
  "output": "static__always_inlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void __ipv4_ct_tuple_reverse (struct ipv4_ct_tuple *tuple)\n",
    "{\n",
    "    __be32 tmp_addr = tuple->saddr;\n",
    "    __be16 tmp;\n",
    "    tuple->saddr = tuple->daddr;\n",
    "    tuple->daddr = tmp_addr;\n",
    "    tmp = tuple->sport;\n",
    "    tuple->sport = tuple->dport;\n",
    "    tuple->dport = tmp;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
__ipv4_ct_tuple_reverse(struct ipv4_ct_tuple *tuple)
{
	__be32 tmp_addr = tuple->saddr;
	__be16 tmp;

	tuple->saddr = tuple->daddr;
	tuple->daddr = tmp_addr;

	tmp = tuple->sport;
	tuple->sport = tuple->dport;
	tuple->dport = tmp;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 507,
  "endLine": 512,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ipv4_ct_tuple_reverse",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv4_ct_tuple *tuple"
  ],
  "output": "static__always_inlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ipv4_ct_tuple_reverse (struct ipv4_ct_tuple *tuple)\n",
    "{\n",
    "    __ipv4_ct_tuple_reverse (tuple);\n",
    "    ct_flip_tuple_dir4 (tuple);\n",
    "}\n"
  ],
  "called_function_list": [
    "ct_flip_tuple_dir4",
    "__ipv4_ct_tuple_reverse"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
ipv4_ct_tuple_reverse(struct ipv4_ct_tuple *tuple)
{
	__ipv4_ct_tuple_reverse(tuple);
	ct_flip_tuple_dir4(tuple);
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "pkt_go_to_next_module",
      "pkt_go_to_next_module": [
        {
          "Project": "cilium",
          "Return Type": "int",
          "Input Params": [],
          "Function Name": "TC_ACT_OK",
          "Return": 0,
          "Description": "will terminate the packet processing pipeline and allows the packet to proceed. Pass the skb onwards either to upper layers of the stack on ingress or down to the networking device driver for transmission on egress, respectively. TC_ACT_OK sets skb->tc_index based on the classid the tc BPF program set. The latter is set out of the tc BPF program itself through skb->tc_classid from the BPF context.",
          "compatible_hookpoints": [
            "xdp",
            "sched_cls",
            "sched_act"
          ],
          "capabilities": [
            "pkt_go_to_next_module"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 514,
  "endLine": 540,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ipv4_ct_extract_l4_ports",
  "developer_inline_comments": [
    {
      "start_line": 524,
      "end_line": 526,
      "text": "/* This function is called from ct_lookup4(), which is sometimes called\n\t * after data has been invalidated (see handle_ipv4_from_lxc())\n\t */"
    },
    {
      "start_line": 534,
      "end_line": 534,
      "text": "/* load sport + dport into tuple */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx",
    " int off",
    " enum ct_dir dir __maybe_unused",
    " struct ipv4_ct_tuple *tuple",
    " bool * has_l4_header __maybe_unused"
  ],
  "output": "static__always_inlineint",
  "helper": [
    "CTX_ACT_OK"
  ],
  "compatibleHookpoints": [
    "xdp",
    "sched_act",
    "sched_cls"
  ],
  "source": [
    "static __always_inline int ipv4_ct_extract_l4_ports (struct  __ctx_buff *ctx, int off, enum ct_dir dir __maybe_unused, struct ipv4_ct_tuple *tuple, bool * has_l4_header __maybe_unused)\n",
    "{\n",
    "\n",
    "#ifdef ENABLE_IPV4_FRAGMENTS\n",
    "    void *data, *data_end;\n",
    "    struct iphdr *ip4;\n",
    "    if (!revalidate_data (ctx, &data, &data_end, &ip4))\n",
    "        return DROP_CT_INVALID_HDR;\n",
    "    return ipv4_handle_fragmentation (ctx, ip4, off, dir, (struct ipv4_frag_l4ports *) &tuple->dport, has_l4_header);\n",
    "\n",
    "#else\n",
    "    if (ctx_load_bytes (ctx, off, &tuple->dport, 4) < 0)\n",
    "        return DROP_CT_INVALID_HDR;\n",
    "\n",
    "#endif\n",
    "    return CTX_ACT_OK;\n",
    "}\n"
  ],
  "called_function_list": [
    "ipv4_handle_fragmentation",
    "revalidate_data",
    "ctx_load_bytes"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int ipv4_ct_extract_l4_ports(struct __ctx_buff *ctx,
						    int off,
						    enum ct_dir dir __maybe_unused,
						    struct ipv4_ct_tuple *tuple,
						    bool *has_l4_header __maybe_unused)
{
#ifdef ENABLE_IPV4_FRAGMENTS
	void *data, *data_end;
	struct iphdr *ip4;

	/* This function is called from ct_lookup4(), which is sometimes called
	 * after data has been invalidated (see handle_ipv4_from_lxc())
	 */
	if (!revalidate_data(ctx, &data, &data_end, &ip4))
		return DROP_CT_INVALID_HDR;

	return ipv4_handle_fragmentation(ctx, ip4, off, dir,
				    (struct ipv4_frag_l4ports *)&tuple->dport,
				    has_l4_header);
#else
	/* load sport + dport into tuple */
	if (ctx_load_bytes(ctx, off, &tuple->dport, 4) < 0)
		return DROP_CT_INVALID_HDR;
#endif

	return CTX_ACT_OK;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 542,
  "endLine": 549,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct4_cilium_dbg_tuple",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx",
    " __u8 type",
    " const struct ipv4_ct_tuple *tuple",
    " __u32 rev_nat_index",
    " int dir"
  ],
  "output": "static__always_inlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct4_cilium_dbg_tuple (struct  __ctx_buff *ctx, __u8 type, const struct ipv4_ct_tuple *tuple, __u32 rev_nat_index, int dir)\n",
    "{\n",
    "    __be32 addr = (dir == CT_INGRESS) ? tuple->saddr : tuple->daddr;\n",
    "    cilium_dbg (ctx, type, addr, rev_nat_index);\n",
    "}\n"
  ],
  "called_function_list": [
    "cilium_dbg"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void ct4_cilium_dbg_tuple(struct __ctx_buff *ctx, __u8 type,
						 const struct ipv4_ct_tuple *tuple,
						 __u32 rev_nat_index, int dir)
{
	__be32 addr = (dir == CT_INGRESS) ? tuple->saddr : tuple->daddr;

	cilium_dbg(ctx, type, addr, rev_nat_index);
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 551,
  "endLine": 606,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_extract_ports4",
  "developer_inline_comments": [
    {
      "start_line": 585,
      "end_line": 585,
      "text": "/* fall through */"
    },
    {
      "start_line": 601,
      "end_line": 601,
      "text": "/* Can't handle extension headers yet */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff *ctx",
    " int off",
    " enum ct_dir dir",
    " struct ipv4_ct_tuple *tuple"
  ],
  "output": "static__always_inlineint",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline int ct_extract_ports4 (struct  __ctx_buff *ctx, int off, enum ct_dir dir, struct ipv4_ct_tuple *tuple)\n",
    "{\n",
    "    int err;\n",
    "    switch (tuple->nexthdr) {\n",
    "    case IPPROTO_ICMP :\n",
    "        if (1) {\n",
    "            __be16 identifier = 0;\n",
    "            __u8 type;\n",
    "            if (ctx_load_bytes (ctx, off, &type, 1) < 0)\n",
    "                return DROP_CT_INVALID_HDR;\n",
    "            if ((type == ICMP_ECHO || type == ICMP_ECHOREPLY) && ctx_load_bytes (ctx, off + offsetof (struct icmphdr, un.echo.id), &identifier, 2) < 0)\n",
    "                return DROP_CT_INVALID_HDR;\n",
    "            tuple->sport = 0;\n",
    "            tuple->dport = 0;\n",
    "            switch (type) {\n",
    "            case ICMP_DEST_UNREACH :\n",
    "            case ICMP_TIME_EXCEEDED :\n",
    "            case ICMP_PARAMETERPROB :\n",
    "                tuple->flags |= TUPLE_F_RELATED;\n",
    "                break;\n",
    "            case ICMP_ECHOREPLY :\n",
    "                tuple->sport = identifier;\n",
    "                break;\n",
    "            case ICMP_ECHO :\n",
    "                tuple->dport = identifier;\n",
    "            default :\n",
    "                break;\n",
    "            }\n",
    "        }\n",
    "        break;\n",
    "    case IPPROTO_TCP :\n",
    "    case IPPROTO_UDP :\n",
    "        err = ipv4_ct_extract_l4_ports (ctx, off, dir, tuple, NULL);\n",
    "        if (err < 0)\n",
    "            return err;\n",
    "        break;\n",
    "    default :\n",
    "        return DROP_CT_UNKNOWN_PROTO;\n",
    "    }\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "ctx_load_bytes",
    "ipv4_ct_extract_l4_ports",
    "offsetof"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int
ct_extract_ports4(struct __ctx_buff *ctx, int off, enum ct_dir dir,
		  struct ipv4_ct_tuple *tuple)
{
	int err;

	switch (tuple->nexthdr) {
	case IPPROTO_ICMP:
		if (1) {
			__be16 identifier = 0;
			__u8 type;

			if (ctx_load_bytes(ctx, off, &type, 1) < 0)
				return DROP_CT_INVALID_HDR;
			if ((type == ICMP_ECHO || type == ICMP_ECHOREPLY) &&
			     ctx_load_bytes(ctx, off + offsetof(struct icmphdr, un.echo.id),
					    &identifier, 2) < 0)
				return DROP_CT_INVALID_HDR;

			tuple->sport = 0;
			tuple->dport = 0;

			switch (type) {
			case ICMP_DEST_UNREACH:
			case ICMP_TIME_EXCEEDED:
			case ICMP_PARAMETERPROB:
				tuple->flags |= TUPLE_F_RELATED;
				break;

			case ICMP_ECHOREPLY:
				tuple->sport = identifier;
				break;
			case ICMP_ECHO:
				tuple->dport = identifier;
				/* fall through */
			default:
				break;
			}
		}
		break;

	case IPPROTO_TCP:
	case IPPROTO_UDP:
		err = ipv4_ct_extract_l4_ports(ctx, off, dir, tuple, NULL);
		if (err < 0)
			return err;

		break;

	default:
		/* Can't handle extension headers yet */
		return DROP_CT_UNKNOWN_PROTO;
	}

	return 0;
}

/* The function determines whether an egress flow identified by the given
 * tuple is a reply.
 *
 * The datapath creates a CT entry in a reverse order. E.g., if a pod sends a
 * request to outside, the CT entry stored in the BPF map will be TUPLE_F_IN:
 * pod => outside. So, we can leverage this fact to determine whether the given
 * flow is a reply.
 */
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 616,
  "endLine": 631,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_is_reply4",
  "developer_inline_comments": [
    {
      "start_line": 608,
      "end_line": 615,
      "text": "/* The function determines whether an egress flow identified by the given\n * tuple is a reply.\n *\n * The datapath creates a CT entry in a reverse order. E.g., if a pod sends a\n * request to outside, the CT entry stored in the BPF map will be TUPLE_F_IN:\n * pod => outside. So, we can leverage this fact to determine whether the given\n * flow is a reply.\n */"
    }
  ],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " struct  __ctx_buff *ctx",
    " int off",
    " struct ipv4_ct_tuple *tuple",
    " bool *is_reply"
  ],
  "output": "static__always_inlineint",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline int ct_is_reply4 (const void *map, struct  __ctx_buff *ctx, int off, struct ipv4_ct_tuple *tuple, bool *is_reply)\n",
    "{\n",
    "    int err = 0;\n",
    "    err = ct_extract_ports4 (ctx, off, CT_EGRESS, tuple);\n",
    "    if (err < 0)\n",
    "        return err;\n",
    "    tuple->flags = TUPLE_F_IN;\n",
    "    *is_reply = map_lookup_elem (map, tuple) != NULL;\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "ct_extract_ports4"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int
ct_is_reply4(const void *map, struct __ctx_buff *ctx, int off,
	     struct ipv4_ct_tuple *tuple, bool *is_reply)
{
	int err = 0;

	err = ct_extract_ports4(ctx, off, CT_EGRESS, tuple);
	if (err < 0)
		return err;

	tuple->flags = TUPLE_F_IN;

	*is_reply = map_lookup_elem(map, tuple) != NULL;

	return 0;
}

/* Offset must point to IPv4 header */
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 634,
  "endLine": 757,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_lookup4",
  "developer_inline_comments": [
    {
      "start_line": 633,
      "end_line": 633,
      "text": "/* Offset must point to IPv4 header */"
    },
    {
      "start_line": 644,
      "end_line": 650,
      "text": "/* The tuple is created in reverse order initially to find a\n\t * potential reverse flow. This is required because the RELATED\n\t * or REPLY state takes precedence over ESTABLISHED due to\n\t * policy requirements.\n\t *\n\t * tuple->flags separates entries that could otherwise be overlapping.\n\t */"
    },
    {
      "start_line": 688,
      "end_line": 688,
      "text": "/* fall through */"
    },
    {
      "start_line": 721,
      "end_line": 721,
      "text": "/* Can't handle extension headers yet */"
    },
    {
      "start_line": 725,
      "end_line": 728,
      "text": "/* Lookup the reverse direction\n\t *\n\t * This will find an existing flow in the reverse direction.\n\t */"
    },
    {
      "start_line": 748,
      "end_line": 748,
      "text": "/* Lookup entry in forward direction */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "const void *map",
    " struct ipv4_ct_tuple *tuple",
    " struct  __ctx_buff *ctx",
    " int off",
    " enum ct_dir dir",
    " struct ct_state *ct_state",
    " __u32 *monitor"
  ],
  "output": "static__always_inlineint",
  "helper": [],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline int ct_lookup4 (const void *map, struct ipv4_ct_tuple *tuple, struct  __ctx_buff *ctx, int off, enum ct_dir dir, struct ct_state *ct_state, __u32 *monitor)\n",
    "{\n",
    "    int err, ret = CT_NEW, action = ACTION_UNSPEC;\n",
    "    bool is_tcp = tuple->nexthdr == IPPROTO_TCP, has_l4_header = true;\n",
    "    union tcp_flags tcp_flags = {\n",
    "        .value = 0}\n",
    "    ;\n",
    "    if (dir == CT_INGRESS)\n",
    "        tuple->flags = TUPLE_F_OUT;\n",
    "    else if (dir == CT_EGRESS)\n",
    "        tuple->flags = TUPLE_F_IN;\n",
    "    else if (dir == CT_SERVICE)\n",
    "        tuple->flags = TUPLE_F_SERVICE;\n",
    "    else\n",
    "        return DROP_CT_INVALID_HDR;\n",
    "    switch (tuple->nexthdr) {\n",
    "    case IPPROTO_ICMP :\n",
    "        if (1) {\n",
    "            __be16 identifier = 0;\n",
    "            __u8 type;\n",
    "            if (ctx_load_bytes (ctx, off, &type, 1) < 0)\n",
    "                return DROP_CT_INVALID_HDR;\n",
    "            if ((type == ICMP_ECHO || type == ICMP_ECHOREPLY) && ctx_load_bytes (ctx, off + offsetof (struct icmphdr, un.echo.id), &identifier, 2) < 0)\n",
    "                return DROP_CT_INVALID_HDR;\n",
    "            tuple->sport = 0;\n",
    "            tuple->dport = 0;\n",
    "            switch (type) {\n",
    "            case ICMP_DEST_UNREACH :\n",
    "            case ICMP_TIME_EXCEEDED :\n",
    "            case ICMP_PARAMETERPROB :\n",
    "                tuple->flags |= TUPLE_F_RELATED;\n",
    "                break;\n",
    "            case ICMP_ECHOREPLY :\n",
    "                tuple->sport = identifier;\n",
    "                break;\n",
    "            case ICMP_ECHO :\n",
    "                tuple->dport = identifier;\n",
    "            default :\n",
    "                action = ACTION_CREATE;\n",
    "                break;\n",
    "            }\n",
    "        }\n",
    "        break;\n",
    "    case IPPROTO_TCP :\n",
    "        err = ipv4_ct_extract_l4_ports (ctx, off, dir, tuple, &has_l4_header);\n",
    "        if (err < 0)\n",
    "            return err;\n",
    "        action = ACTION_CREATE;\n",
    "        if (has_l4_header) {\n",
    "            if (ctx_load_bytes (ctx, off + 12, &tcp_flags, 2) < 0)\n",
    "                return DROP_CT_INVALID_HDR;\n",
    "            if (unlikely (tcp_flags.value & (TCP_FLAG_RST | TCP_FLAG_FIN)))\n",
    "                action = ACTION_CLOSE;\n",
    "        }\n",
    "        break;\n",
    "    case IPPROTO_UDP :\n",
    "        err = ipv4_ct_extract_l4_ports (ctx, off, dir, tuple, NULL);\n",
    "        if (err < 0)\n",
    "            return err;\n",
    "        action = ACTION_CREATE;\n",
    "        break;\n",
    "    default :\n",
    "        return DROP_CT_UNKNOWN_PROTO;\n",
    "    }\n",
    "\n",
    "#ifndef QUIET_CT\n",
    "    cilium_dbg3 (ctx, DBG_CT_LOOKUP4_1, tuple->saddr, tuple->daddr, (bpf_ntohs (tuple->sport) << 16) | bpf_ntohs (tuple->dport));\n",
    "    cilium_dbg3 (ctx, DBG_CT_LOOKUP4_2, (tuple->nexthdr << 8) | tuple->flags, 0, 0);\n",
    "\n",
    "#endif\n",
    "    ret = __ct_lookup (map, ctx, tuple, action, dir, ct_state, is_tcp, tcp_flags, monitor);\n",
    "    if (ret != CT_NEW) {\n",
    "        if (likely (ret == CT_ESTABLISHED || ret == CT_REOPENED)) {\n",
    "            if (unlikely (tuple->flags & TUPLE_F_RELATED))\n",
    "                ret = CT_RELATED;\n",
    "            else\n",
    "                ret = CT_REPLY;\n",
    "        }\n",
    "        goto out;\n",
    "    }\n",
    "    relax_verifier ();\n",
    "    if (dir != CT_SERVICE) {\n",
    "        ipv4_ct_tuple_reverse (tuple);\n",
    "        ret = __ct_lookup (map, ctx, tuple, action, dir, ct_state, is_tcp, tcp_flags, monitor);\n",
    "    }\n",
    "out :\n",
    "    cilium_dbg (ctx, DBG_CT_VERDICT, ret < 0 ? -ret : ret, ct_state->rev_nat_index);\n",
    "    return ret;\n",
    "}\n"
  ],
  "called_function_list": [
    "cilium_dbg",
    "unlikely",
    "likely",
    "ipv4_ct_extract_l4_ports",
    "ipv4_ct_tuple_reverse",
    "ctx_load_bytes",
    "offsetof",
    "__ct_lookup",
    "cilium_dbg3",
    "bpf_ntohs",
    "relax_verifier"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int ct_lookup4(const void *map,
				      struct ipv4_ct_tuple *tuple,
				      struct __ctx_buff *ctx, int off, enum ct_dir dir,
				      struct ct_state *ct_state, __u32 *monitor)
{
	int err, ret = CT_NEW, action = ACTION_UNSPEC;
	bool is_tcp = tuple->nexthdr == IPPROTO_TCP,
	     has_l4_header = true;
	union tcp_flags tcp_flags = { .value = 0 };

	/* The tuple is created in reverse order initially to find a
	 * potential reverse flow. This is required because the RELATED
	 * or REPLY state takes precedence over ESTABLISHED due to
	 * policy requirements.
	 *
	 * tuple->flags separates entries that could otherwise be overlapping.
	 */
	if (dir == CT_INGRESS)
		tuple->flags = TUPLE_F_OUT;
	else if (dir == CT_EGRESS)
		tuple->flags = TUPLE_F_IN;
	else if (dir == CT_SERVICE)
		tuple->flags = TUPLE_F_SERVICE;
	else
		return DROP_CT_INVALID_HDR;

	switch (tuple->nexthdr) {
	case IPPROTO_ICMP:
		if (1) {
			__be16 identifier = 0;
			__u8 type;

			if (ctx_load_bytes(ctx, off, &type, 1) < 0)
				return DROP_CT_INVALID_HDR;
			if ((type == ICMP_ECHO || type == ICMP_ECHOREPLY) &&
			     ctx_load_bytes(ctx, off + offsetof(struct icmphdr, un.echo.id),
					    &identifier, 2) < 0)
				return DROP_CT_INVALID_HDR;

			tuple->sport = 0;
			tuple->dport = 0;

			switch (type) {
			case ICMP_DEST_UNREACH:
			case ICMP_TIME_EXCEEDED:
			case ICMP_PARAMETERPROB:
				tuple->flags |= TUPLE_F_RELATED;
				break;

			case ICMP_ECHOREPLY:
				tuple->sport = identifier;
				break;
			case ICMP_ECHO:
				tuple->dport = identifier;
				/* fall through */
			default:
				action = ACTION_CREATE;
				break;
			}
		}
		break;

	case IPPROTO_TCP:
		err = ipv4_ct_extract_l4_ports(ctx, off, dir, tuple, &has_l4_header);
		if (err < 0)
			return err;

		action = ACTION_CREATE;

		if (has_l4_header) {
			if (ctx_load_bytes(ctx, off + 12, &tcp_flags, 2) < 0)
				return DROP_CT_INVALID_HDR;

			if (unlikely(tcp_flags.value & (TCP_FLAG_RST|TCP_FLAG_FIN)))
				action = ACTION_CLOSE;
		}
		break;

	case IPPROTO_UDP:
		err = ipv4_ct_extract_l4_ports(ctx, off, dir, tuple, NULL);
		if (err < 0)
			return err;

		action = ACTION_CREATE;
		break;

	default:
		/* Can't handle extension headers yet */
		return DROP_CT_UNKNOWN_PROTO;
	}

	/* Lookup the reverse direction
	 *
	 * This will find an existing flow in the reverse direction.
	 */
#ifndef QUIET_CT
	cilium_dbg3(ctx, DBG_CT_LOOKUP4_1, tuple->saddr, tuple->daddr,
		      (bpf_ntohs(tuple->sport) << 16) | bpf_ntohs(tuple->dport));
	cilium_dbg3(ctx, DBG_CT_LOOKUP4_2, (tuple->nexthdr << 8) | tuple->flags, 0, 0);
#endif
	ret = __ct_lookup(map, ctx, tuple, action, dir, ct_state, is_tcp,
			  tcp_flags, monitor);
	if (ret != CT_NEW) {
		if (likely(ret == CT_ESTABLISHED || ret == CT_REOPENED)) {
			if (unlikely(tuple->flags & TUPLE_F_RELATED))
				ret = CT_RELATED;
			else
				ret = CT_REPLY;
		}
		goto out;
	}

	relax_verifier();

	/* Lookup entry in forward direction */
	if (dir != CT_SERVICE) {
		ipv4_ct_tuple_reverse(tuple);
		ret = __ct_lookup(map, ctx, tuple, action, dir, ct_state,
				  is_tcp, tcp_flags, monitor);
	}
out:
	cilium_dbg(ctx, DBG_CT_VERDICT, ret < 0 ? -ret : ret, ct_state->rev_nat_index);
	return ret;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 759,
  "endLine": 770,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_update6_backend_id",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " const struct ipv6_ct_tuple *tuple",
    " const struct ct_state *state"
  ],
  "output": "static__always_inlinevoid",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_update6_backend_id (const void *map, const struct ipv6_ct_tuple *tuple, const struct ct_state *state)\n",
    "{\n",
    "    struct ct_entry *entry;\n",
    "    entry = map_lookup_elem (map, tuple);\n",
    "    if (!entry)\n",
    "        return;\n",
    "    entry->backend_id = state->backend_id;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
ct_update6_backend_id(const void *map, const struct ipv6_ct_tuple *tuple,
		      const struct ct_state *state)
{
	struct ct_entry *entry;

	entry = map_lookup_elem(map, tuple);
	if (!entry)
		return;

	entry->backend_id = state->backend_id;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 772,
  "endLine": 783,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_update6_rev_nat_index",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " const struct ipv6_ct_tuple *tuple",
    " const struct ct_state *state"
  ],
  "output": "static__always_inlinevoid",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_update6_rev_nat_index (const void *map, const struct ipv6_ct_tuple *tuple, const struct ct_state *state)\n",
    "{\n",
    "    struct ct_entry *entry;\n",
    "    entry = map_lookup_elem (map, tuple);\n",
    "    if (!entry)\n",
    "        return;\n",
    "    entry->rev_nat_index = state->rev_nat_index;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
ct_update6_rev_nat_index(const void *map, const struct ipv6_ct_tuple *tuple,
			 const struct ct_state *state)
{
	struct ct_entry *entry;

	entry = map_lookup_elem(map, tuple);
	if (!entry)
		return;

	entry->rev_nat_index = state->rev_nat_index;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 785,
  "endLine": 796,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_update6_dsr",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " const struct ipv6_ct_tuple *tuple",
    " const bool dsr"
  ],
  "output": "static__always_inlinevoid",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_update6_dsr (const void *map, const struct ipv6_ct_tuple *tuple, const bool dsr)\n",
    "{\n",
    "    struct ct_entry *entry;\n",
    "    entry = map_lookup_elem (map, tuple);\n",
    "    if (!entry)\n",
    "        return;\n",
    "    entry->dsr = dsr;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
ct_update6_dsr(const void *map, const struct ipv6_ct_tuple *tuple,
	       const bool dsr)
{
	struct ct_entry *entry;

	entry = map_lookup_elem(map, tuple);
	if (!entry)
		return;

	entry->dsr = dsr;
}

/* Offset must point to IPv6 */
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 799,
  "endLine": 865,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_create6",
  "developer_inline_comments": [
    {
      "start_line": 798,
      "end_line": 798,
      "text": "/* Offset must point to IPv6 */"
    },
    {
      "start_line": 805,
      "end_line": 805,
      "text": "/* Create entry in original direction */"
    },
    {
      "start_line": 810,
      "end_line": 812,
      "text": "/* Note if this is a proxy connection so that replies can be redirected\n\t * back to the proxy.\n\t */"
    },
    {
      "start_line": 846,
      "end_line": 846,
      "text": "/* Create an ICMPv6 entry to relate errors */"
    },
    {
      "start_line": 854,
      "end_line": 854,
      "text": "/* For ICMP, there is no SYN. */"
    }
  ],
  "updateMaps": [
    " map_related",
    " map_main"
  ],
  "readMaps": [],
  "input": [
    "const void *map_main",
    " const void *map_related",
    " struct ipv6_ct_tuple *tuple",
    " struct  __ctx_buff *ctx",
    " const int dir",
    " const struct ct_state *ct_state",
    " bool proxy_redirect",
    " bool from_l7lb"
  ],
  "output": "static__always_inlineint",
  "helper": [
    "send_signal"
  ],
  "compatibleHookpoints": [
    "raw_tracepoint_writable",
    "perf_event",
    "tracepoint",
    "kprobe",
    "raw_tracepoint"
  ],
  "source": [
    "static __always_inline int ct_create6 (const void *map_main, const void *map_related, struct ipv6_ct_tuple *tuple, struct  __ctx_buff *ctx, const int dir, const struct ct_state *ct_state, bool proxy_redirect, bool from_l7lb)\n",
    "{\n",
    "    struct ct_entry entry = {}\n",
    "    ;\n",
    "    bool is_tcp = tuple->nexthdr == IPPROTO_TCP;\n",
    "    union tcp_flags seen_flags = {\n",
    "        .value = 0}\n",
    "    ;\n",
    "    entry.proxy_redirect = proxy_redirect;\n",
    "    entry.from_l7lb = from_l7lb;\n",
    "    if (dir == CT_SERVICE)\n",
    "        entry.backend_id = ct_state->backend_id;\n",
    "    entry.lb_loopback = ct_state->loopback;\n",
    "    entry.node_port = ct_state->node_port;\n",
    "    relax_verifier ();\n",
    "    entry.dsr = ct_state->dsr;\n",
    "    entry.ifindex = ct_state->ifindex;\n",
    "    entry.rev_nat_index = ct_state->rev_nat_index;\n",
    "    seen_flags.value |= is_tcp ? TCP_FLAG_SYN : 0;\n",
    "    ct_update_timeout (&entry, is_tcp, dir, seen_flags);\n",
    "    if (dir == CT_INGRESS) {\n",
    "        entry.rx_packets = 1;\n",
    "        entry.rx_bytes = ctx_full_len (ctx);\n",
    "    }\n",
    "    else if (dir == CT_EGRESS) {\n",
    "        entry.tx_packets = 1;\n",
    "        entry.tx_bytes = ctx_full_len (ctx);\n",
    "    }\n",
    "    cilium_dbg3 (ctx, DBG_CT_CREATED6, entry.rev_nat_index, ct_state->src_sec_id, 0);\n",
    "    entry.src_sec_id = ct_state->src_sec_id;\n",
    "    if (map_update_elem (map_main, tuple, &entry, 0) < 0) {\n",
    "        send_signal_ct_fill_up (ctx, SIGNAL_PROTO_V6);\n",
    "        return DROP_CT_CREATE_FAILED;\n",
    "    }\n",
    "    if (map_related != NULL) {\n",
    "        struct ipv6_ct_tuple icmp_tuple = {\n",
    "            .nexthdr = IPPROTO_ICMPV6,\n",
    "            .sport = 0,\n",
    "            .dport = 0,\n",
    "            .flags = tuple->flags | TUPLE_F_RELATED,}\n",
    "        ;\n",
    "        entry.seen_non_syn = true;\n",
    "        ipv6_addr_copy (&icmp_tuple.daddr, &tuple->daddr);\n",
    "        ipv6_addr_copy (&icmp_tuple.saddr, &tuple->saddr);\n",
    "        if (map_update_elem (map_related, &icmp_tuple, &entry, 0) < 0) {\n",
    "            send_signal_ct_fill_up (ctx, SIGNAL_PROTO_V6);\n",
    "            return DROP_CT_CREATE_FAILED;\n",
    "        }\n",
    "    }\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "ipv6_addr_copy",
    "ctx_full_len",
    "send_signal_ct_fill_up",
    "cilium_dbg3",
    "ct_update_timeout",
    "relax_verifier"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int ct_create6(const void *map_main, const void *map_related,
				      struct ipv6_ct_tuple *tuple,
				      struct __ctx_buff *ctx, const int dir,
				      const struct ct_state *ct_state,
				      bool proxy_redirect, bool from_l7lb)
{
	/* Create entry in original direction */
	struct ct_entry entry = { };
	bool is_tcp = tuple->nexthdr == IPPROTO_TCP;
	union tcp_flags seen_flags = { .value = 0 };

	/* Note if this is a proxy connection so that replies can be redirected
	 * back to the proxy.
	 */
	entry.proxy_redirect = proxy_redirect;
	entry.from_l7lb = from_l7lb;

	if (dir == CT_SERVICE)
		entry.backend_id = ct_state->backend_id;

	entry.lb_loopback = ct_state->loopback;
	entry.node_port = ct_state->node_port;
	relax_verifier();
	entry.dsr = ct_state->dsr;
	entry.ifindex = ct_state->ifindex;

	entry.rev_nat_index = ct_state->rev_nat_index;
	seen_flags.value |= is_tcp ? TCP_FLAG_SYN : 0;
	ct_update_timeout(&entry, is_tcp, dir, seen_flags);

	if (dir == CT_INGRESS) {
		entry.rx_packets = 1;
		entry.rx_bytes = ctx_full_len(ctx);
	} else if (dir == CT_EGRESS) {
		entry.tx_packets = 1;
		entry.tx_bytes = ctx_full_len(ctx);
	}

	cilium_dbg3(ctx, DBG_CT_CREATED6, entry.rev_nat_index, ct_state->src_sec_id, 0);

	entry.src_sec_id = ct_state->src_sec_id;
	if (map_update_elem(map_main, tuple, &entry, 0) < 0) {
		send_signal_ct_fill_up(ctx, SIGNAL_PROTO_V6);
		return DROP_CT_CREATE_FAILED;
	}

	if (map_related != NULL) {
		/* Create an ICMPv6 entry to relate errors */
		struct ipv6_ct_tuple icmp_tuple = {
			.nexthdr = IPPROTO_ICMPV6,
			.sport = 0,
			.dport = 0,
			.flags = tuple->flags | TUPLE_F_RELATED,
		};

		entry.seen_non_syn = true; /* For ICMP, there is no SYN. */

		ipv6_addr_copy(&icmp_tuple.daddr, &tuple->daddr);
		ipv6_addr_copy(&icmp_tuple.saddr, &tuple->saddr);

		if (map_update_elem(map_related, &icmp_tuple, &entry, 0) < 0) {
			send_signal_ct_fill_up(ctx, SIGNAL_PROTO_V6);
			return DROP_CT_CREATE_FAILED;
		}
	}
	return 0;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 867,
  "endLine": 878,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_update4_backend_id",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " const struct ipv4_ct_tuple *tuple",
    " const struct ct_state *state"
  ],
  "output": "static__always_inlinevoid",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_update4_backend_id (const void *map, const struct ipv4_ct_tuple *tuple, const struct ct_state *state)\n",
    "{\n",
    "    struct ct_entry *entry;\n",
    "    entry = map_lookup_elem (map, tuple);\n",
    "    if (!entry)\n",
    "        return;\n",
    "    entry->backend_id = state->backend_id;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void ct_update4_backend_id(const void *map,
						  const struct ipv4_ct_tuple *tuple,
						  const struct ct_state *state)
{
	struct ct_entry *entry;

	entry = map_lookup_elem(map, tuple);
	if (!entry)
		return;

	entry->backend_id = state->backend_id;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 880,
  "endLine": 891,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_update4_rev_nat_index",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " const struct ipv4_ct_tuple *tuple",
    " const struct ct_state *state"
  ],
  "output": "static__always_inlinevoid",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_update4_rev_nat_index (const void *map, const struct ipv4_ct_tuple *tuple, const struct ct_state *state)\n",
    "{\n",
    "    struct ct_entry *entry;\n",
    "    entry = map_lookup_elem (map, tuple);\n",
    "    if (!entry)\n",
    "        return;\n",
    "    entry->rev_nat_index = state->rev_nat_index;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
ct_update4_rev_nat_index(const void *map, const struct ipv4_ct_tuple *tuple,
			 const struct ct_state *state)
{
	struct ct_entry *entry;

	entry = map_lookup_elem(map, tuple);
	if (!entry)
		return;

	entry->rev_nat_index = state->rev_nat_index;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 893,
  "endLine": 904,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_update4_dsr",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " const struct ipv4_ct_tuple *tuple",
    " const bool dsr"
  ],
  "output": "static__always_inlinevoid",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_update4_dsr (const void *map, const struct ipv4_ct_tuple *tuple, const bool dsr)\n",
    "{\n",
    "    struct ct_entry *entry;\n",
    "    entry = map_lookup_elem (map, tuple);\n",
    "    if (!entry)\n",
    "        return;\n",
    "    entry->dsr = dsr;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
ct_update4_dsr(const void *map, const struct ipv4_ct_tuple *tuple,
	       const bool dsr)
{
	struct ct_entry *entry;

	entry = map_lookup_elem(map, tuple);
	if (!entry)
		return;

	entry->dsr = dsr;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 906,
  "endLine": 1006,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_create4",
  "developer_inline_comments": [
    {
      "start_line": 913,
      "end_line": 913,
      "text": "/* Create entry in original direction */"
    },
    {
      "start_line": 918,
      "end_line": 920,
      "text": "/* Note if this is a proxy connection so that replies can be redirected\n\t * back to the proxy.\n\t */"
    },
    {
      "start_line": 960,
      "end_line": 964,
      "text": "/* We are looping back into the origin endpoint through a\n\t\t * service, set up a conntrack tuple for the reply to ensure we\n\t\t * do rev NAT before attempting to route the destination\n\t\t * address which will not point back to the right source.\n\t\t */"
    },
    {
      "start_line": 985,
      "end_line": 985,
      "text": "/* Create an ICMP entry to relate errors */"
    },
    {
      "start_line": 995,
      "end_line": 995,
      "text": "/* For ICMP, there is no SYN. */"
    },
    {
      "start_line": 996,
      "end_line": 999,
      "text": "/* Previous map update succeeded, we could delete it in case\n\t\t * the below throws an error, but we might as well just let\n\t\t * it time out.\n\t\t */"
    }
  ],
  "updateMaps": [
    " map_related",
    " map_main"
  ],
  "readMaps": [],
  "input": [
    "const void *map_main",
    " const void *map_related",
    " struct ipv4_ct_tuple *tuple",
    " struct  __ctx_buff *ctx",
    " const int dir",
    " const struct ct_state *ct_state",
    " bool proxy_redirect",
    " bool from_l7lb"
  ],
  "output": "static__always_inlineint",
  "helper": [
    "send_signal"
  ],
  "compatibleHookpoints": [
    "raw_tracepoint_writable",
    "perf_event",
    "tracepoint",
    "kprobe",
    "raw_tracepoint"
  ],
  "source": [
    "static __always_inline int ct_create4 (const void *map_main, const void *map_related, struct ipv4_ct_tuple *tuple, struct  __ctx_buff *ctx, const int dir, const struct ct_state *ct_state, bool proxy_redirect, bool from_l7lb)\n",
    "{\n",
    "    struct ct_entry entry = {}\n",
    "    ;\n",
    "    bool is_tcp = tuple->nexthdr == IPPROTO_TCP;\n",
    "    union tcp_flags seen_flags = {\n",
    "        .value = 0}\n",
    "    ;\n",
    "    entry.proxy_redirect = proxy_redirect;\n",
    "    entry.from_l7lb = from_l7lb;\n",
    "    entry.lb_loopback = ct_state->loopback;\n",
    "    entry.node_port = ct_state->node_port;\n",
    "    relax_verifier ();\n",
    "    entry.dsr = ct_state->dsr;\n",
    "    entry.ifindex = ct_state->ifindex;\n",
    "    if (dir == CT_SERVICE)\n",
    "        entry.backend_id = ct_state->backend_id;\n",
    "    entry.rev_nat_index = ct_state->rev_nat_index;\n",
    "    seen_flags.value |= is_tcp ? TCP_FLAG_SYN : 0;\n",
    "    ct_update_timeout (&entry, is_tcp, dir, seen_flags);\n",
    "    if (dir == CT_INGRESS) {\n",
    "        entry.rx_packets = 1;\n",
    "        entry.rx_bytes = ctx_full_len (ctx);\n",
    "    }\n",
    "    else if (dir == CT_EGRESS) {\n",
    "        entry.tx_packets = 1;\n",
    "        entry.tx_bytes = ctx_full_len (ctx);\n",
    "    }\n",
    "    cilium_dbg3 (ctx, DBG_CT_CREATED4, entry.rev_nat_index, ct_state->src_sec_id, ct_state->addr);\n",
    "    entry.src_sec_id = ct_state->src_sec_id;\n",
    "    if (map_update_elem (map_main, tuple, &entry, 0) < 0) {\n",
    "        send_signal_ct_fill_up (ctx, SIGNAL_PROTO_V4);\n",
    "        return DROP_CT_CREATE_FAILED;\n",
    "    }\n",
    "    if (ct_state->addr && ct_state->loopback) {\n",
    "        __u8 flags = tuple->flags;\n",
    "        __be32 saddr, daddr;\n",
    "        saddr = tuple->saddr;\n",
    "        daddr = tuple->daddr;\n",
    "        tuple->flags = TUPLE_F_IN;\n",
    "        if (dir == CT_INGRESS) {\n",
    "            tuple->saddr = ct_state->addr;\n",
    "            tuple->daddr = ct_state->svc_addr;\n",
    "        }\n",
    "        else {\n",
    "            tuple->saddr = ct_state->svc_addr;\n",
    "            tuple->daddr = ct_state->addr;\n",
    "        }\n",
    "        if (map_update_elem (map_main, tuple, &entry, 0) < 0) {\n",
    "            send_signal_ct_fill_up (ctx, SIGNAL_PROTO_V4);\n",
    "            return DROP_CT_CREATE_FAILED;\n",
    "        }\n",
    "        tuple->saddr = saddr;\n",
    "        tuple->daddr = daddr;\n",
    "        tuple->flags = flags;\n",
    "    }\n",
    "    if (map_related != NULL) {\n",
    "        struct ipv4_ct_tuple icmp_tuple = {\n",
    "            .daddr = tuple->daddr,\n",
    "            .saddr = tuple->saddr,\n",
    "            .nexthdr = IPPROTO_ICMP,\n",
    "            .sport = 0,\n",
    "            .dport = 0,\n",
    "            .flags = tuple->flags | TUPLE_F_RELATED,}\n",
    "        ;\n",
    "        entry.seen_non_syn = true;\n",
    "        if (map_update_elem (map_related, &icmp_tuple, &entry, 0) < 0) {\n",
    "            send_signal_ct_fill_up (ctx, SIGNAL_PROTO_V4);\n",
    "            return DROP_CT_CREATE_FAILED;\n",
    "        }\n",
    "    }\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "ctx_full_len",
    "send_signal_ct_fill_up",
    "cilium_dbg3",
    "ct_update_timeout",
    "relax_verifier"
  ],
  "call_depth": -1,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline int ct_create4(const void *map_main,
				      const void *map_related,
				      struct ipv4_ct_tuple *tuple,
				      struct __ctx_buff *ctx, const int dir,
				      const struct ct_state *ct_state,
				      bool proxy_redirect, bool from_l7lb)
{
	/* Create entry in original direction */
	struct ct_entry entry = { };
	bool is_tcp = tuple->nexthdr == IPPROTO_TCP;
	union tcp_flags seen_flags = { .value = 0 };

	/* Note if this is a proxy connection so that replies can be redirected
	 * back to the proxy.
	 */
	entry.proxy_redirect = proxy_redirect;
	entry.from_l7lb = from_l7lb;

	entry.lb_loopback = ct_state->loopback;
	entry.node_port = ct_state->node_port;
	relax_verifier();
	entry.dsr = ct_state->dsr;
	entry.ifindex = ct_state->ifindex;

	if (dir == CT_SERVICE)
		entry.backend_id = ct_state->backend_id;
	entry.rev_nat_index = ct_state->rev_nat_index;
	seen_flags.value |= is_tcp ? TCP_FLAG_SYN : 0;
	ct_update_timeout(&entry, is_tcp, dir, seen_flags);

	if (dir == CT_INGRESS) {
		entry.rx_packets = 1;
		entry.rx_bytes = ctx_full_len(ctx);
	} else if (dir == CT_EGRESS) {
		entry.tx_packets = 1;
		entry.tx_bytes = ctx_full_len(ctx);
	}

	cilium_dbg3(ctx, DBG_CT_CREATED4, entry.rev_nat_index,
		    ct_state->src_sec_id, ct_state->addr);

	entry.src_sec_id = ct_state->src_sec_id;
	if (map_update_elem(map_main, tuple, &entry, 0) < 0) {
		send_signal_ct_fill_up(ctx, SIGNAL_PROTO_V4);
		return DROP_CT_CREATE_FAILED;
	}

	if (ct_state->addr && ct_state->loopback) {
		__u8 flags = tuple->flags;
		__be32 saddr, daddr;

		saddr = tuple->saddr;
		daddr = tuple->daddr;

		/* We are looping back into the origin endpoint through a
		 * service, set up a conntrack tuple for the reply to ensure we
		 * do rev NAT before attempting to route the destination
		 * address which will not point back to the right source.
		 */
		tuple->flags = TUPLE_F_IN;
		if (dir == CT_INGRESS) {
			tuple->saddr = ct_state->addr;
			tuple->daddr = ct_state->svc_addr;
		} else {
			tuple->saddr = ct_state->svc_addr;
			tuple->daddr = ct_state->addr;
		}

		if (map_update_elem(map_main, tuple, &entry, 0) < 0) {
			send_signal_ct_fill_up(ctx, SIGNAL_PROTO_V4);
			return DROP_CT_CREATE_FAILED;
		}

		tuple->saddr = saddr;
		tuple->daddr = daddr;
		tuple->flags = flags;
	}

	if (map_related != NULL) {
		/* Create an ICMP entry to relate errors */
		struct ipv4_ct_tuple icmp_tuple = {
			.daddr = tuple->daddr,
			.saddr = tuple->saddr,
			.nexthdr = IPPROTO_ICMP,
			.sport = 0,
			.dport = 0,
			.flags = tuple->flags | TUPLE_F_RELATED,
		};

		entry.seen_non_syn = true; /* For ICMP, there is no SYN. */
		/* Previous map update succeeded, we could delete it in case
		 * the below throws an error, but we might as well just let
		 * it time out.
		 */
		if (map_update_elem(map_related, &icmp_tuple, &entry, 0) < 0) {
			send_signal_ct_fill_up(ctx, SIGNAL_PROTO_V4);
			return DROP_CT_CREATE_FAILED;
		}
	}
	return 0;
}

/* The function tries to determine whether the flow identified by the given
 * CT_INGRESS tuple belongs to a NodePort traffic (i.e., outside client => N/S
 * LB => local backend).
 *
 * When the client send the NodePort request, the NodePort BPF
 * (nodeport_lb{4,6}()) creates the CT_EGRESS entry for the
 * (saddr=client,daddr=backend) tuple. So, to derive whether the reply packet
 * backend => client belongs to the LB flow we can query the CT_EGRESS entry.
 */
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 1017,
  "endLine": 1032,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_has_nodeport_egress_entry4",
  "developer_inline_comments": [
    {
      "start_line": 1008,
      "end_line": 1016,
      "text": "/* The function tries to determine whether the flow identified by the given\n * CT_INGRESS tuple belongs to a NodePort traffic (i.e., outside client => N/S\n * LB => local backend).\n *\n * When the client send the NodePort request, the NodePort BPF\n * (nodeport_lb{4,6}()) creates the CT_EGRESS entry for the\n * (saddr=client,daddr=backend) tuple. So, to derive whether the reply packet\n * backend => client belongs to the LB flow we can query the CT_EGRESS entry.\n */"
    }
  ],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " struct ipv4_ct_tuple *ingress_tuple"
  ],
  "output": "static__always_inlinebool",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline bool ct_has_nodeport_egress_entry4 (const void *map, struct ipv4_ct_tuple *ingress_tuple)\n",
    "{\n",
    "    __u8 prev_flags = ingress_tuple->flags;\n",
    "    struct ct_entry *entry;\n",
    "    ingress_tuple->flags = TUPLE_F_OUT;\n",
    "    entry = map_lookup_elem (map, ingress_tuple);\n",
    "    ingress_tuple->flags = prev_flags;\n",
    "    if (entry)\n",
    "        return entry->node_port;\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline bool
ct_has_nodeport_egress_entry4(const void *map,
			      struct ipv4_ct_tuple *ingress_tuple)
{
	__u8 prev_flags = ingress_tuple->flags;
	struct ct_entry *entry;

	ingress_tuple->flags = TUPLE_F_OUT;
	entry = map_lookup_elem(map, ingress_tuple);
	ingress_tuple->flags = prev_flags;

	if (entry)
		return entry->node_port;

	return 0;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 1034,
  "endLine": 1049,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_has_nodeport_egress_entry6",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " struct ipv6_ct_tuple *ingress_tuple"
  ],
  "output": "static__always_inlinebool",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline bool ct_has_nodeport_egress_entry6 (const void *map, struct ipv6_ct_tuple *ingress_tuple)\n",
    "{\n",
    "    __u8 prev_flags = ingress_tuple->flags;\n",
    "    struct ct_entry *entry;\n",
    "    ingress_tuple->flags = TUPLE_F_OUT;\n",
    "    entry = map_lookup_elem (map, ingress_tuple);\n",
    "    ingress_tuple->flags = prev_flags;\n",
    "    if (entry)\n",
    "        return entry->node_port;\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline bool
ct_has_nodeport_egress_entry6(const void *map,
			      struct ipv6_ct_tuple *ingress_tuple)
{
	__u8 prev_flags = ingress_tuple->flags;
	struct ct_entry *entry;

	ingress_tuple->flags = TUPLE_F_OUT;
	entry = map_lookup_elem(map, ingress_tuple);
	ingress_tuple->flags = prev_flags;

	if (entry)
		return entry->node_port;

	return 0;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "map_read",
      "map_read": [
        {
          "Project": "cilium",
          "Return Type": "void*",
          "Description": "Perform a lookup in <[ map ]>(IP: 0) for an entry associated to key. ",
          "Return": " Map value associated to key, or NULL if no entry was found.",
          "Function Name": "map_lookup_elem",
          "Input Params": [
            "{Type: struct map ,Var: *map}",
            "{Type:  const void ,Var: *key}"
          ],
          "compatible_hookpoints": [
            "socket_filter",
            "kprobe",
            "sched_cls",
            "sched_act",
            "tracepoint",
            "xdp",
            "perf_event",
            "cgroup_skb",
            "cgroup_sock",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "sock_ops",
            "sk_skb",
            "cgroup_device",
            "sk_msg",
            "raw_tracepoint",
            "cgroup_sock_addr",
            "lwt_seg6local",
            "sk_reuseport",
            "flow_dissector",
            "cgroup_sysctl",
            "raw_tracepoint_writable"
          ],
          "capabilities": [
            "map_read"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 1051,
  "endLine": 1061,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/conntrack.h",
  "funcName": "ct_update_nodeport",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [
    " map"
  ],
  "input": [
    "const void *map",
    " const void *tuple",
    " const bool node_port"
  ],
  "output": "static__always_inlinevoid",
  "helper": [
    "map_lookup_elem"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "lwt_out",
    "lwt_seg6local",
    "sk_skb",
    "sched_cls",
    "socket_filter",
    "sk_reuseport",
    "sk_msg",
    "kprobe",
    "xdp",
    "cgroup_skb",
    "raw_tracepoint_writable",
    "lwt_in",
    "perf_event",
    "cgroup_sock",
    "cgroup_sock_addr",
    "raw_tracepoint",
    "flow_dissector",
    "cgroup_sysctl",
    "sock_ops",
    "tracepoint",
    "sched_act",
    "cgroup_device"
  ],
  "source": [
    "static __always_inline void ct_update_nodeport (const void *map, const void *tuple, const bool node_port)\n",
    "{\n",
    "    struct ct_entry *entry;\n",
    "    entry = map_lookup_elem (map, tuple);\n",
    "    if (!entry)\n",
    "        return;\n",
    "    entry->node_port = node_port;\n",
    "}\n"
  ],
  "called_function_list": [],
  "call_depth": 0,
  "humanFuncDescription": [
    {}
  ],
  "AI_func_description": [
    {
      "description": "",
      "author": "",
      "authorEmail": "",
      "date": "",
      "invocationParameters": ""
    }
  ]
} 
 OPENED COMMENT END 
 */ 
static __always_inline void
ct_update_nodeport(const void *map, const void *tuple, const bool node_port)
{
	struct ct_entry *entry;

	entry = map_lookup_elem(map, tuple);
	if (!entry)
		return;

	entry->node_port = node_port;
}
#endif /* __LIB_CONNTRACK_H_ */