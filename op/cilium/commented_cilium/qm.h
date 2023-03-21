/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/* Copyright Authors of Cilium */

#ifndef __QM_H_
#define __QM_H_

#include <bpf/ctx/ctx.h>

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 9,
  "endLine": 22,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/cilium/lib/qm.h",
  "funcName": "reset_queue_mapping",
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
    },
    {
      "start_line": 12,
      "end_line": 19,
      "text": "/* Workaround for GH-18311 where veth driver might have recorded\n\t * veth's RX queue mapping instead of leaving it at 0. This can\n\t * cause issues on the phys device where all traffic would only\n\t * hit a single TX queue (given veth device had a single one and\n\t * mapping was left at 1). Reset so that stack picks a fresh queue.\n\t * Kernel fix is at 710ad98c363a (\"veth: Do not record rx queue\n\t * hint in veth_xmit\").\n\t */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct  __ctx_buff * ctx __maybe_unused"
  ],
  "output": "staticinlinevoid",
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
    "static inline void reset_queue_mapping (struct  __ctx_buff * ctx __maybe_unused)\n",
    "{\n",
    "\n",
    "#if defined(RESET_QUEUES) && __ctx_is == __ctx_skb\n",
    "    ctx->queue_mapping = 0;\n",
    "\n",
    "#endif\n",
    "}\n"
  ],
  "called_function_list": [
    "defined"
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
static inline void reset_queue_mapping(struct __ctx_buff *ctx __maybe_unused)
{
#if defined(RESET_QUEUES) && __ctx_is == __ctx_skb
	/* Workaround for GH-18311 where veth driver might have recorded
	 * veth's RX queue mapping instead of leaving it at 0. This can
	 * cause issues on the phys device where all traffic would only
	 * hit a single TX queue (given veth device had a single one and
	 * mapping was left at 1). Reset so that stack picks a fresh queue.
	 * Kernel fix is at 710ad98c363a ("veth: Do not record rx queue
	 * hint in veth_xmit").
	 */
	ctx->queue_mapping = 0;
#endif
}

#endif /* __QM_H_ */