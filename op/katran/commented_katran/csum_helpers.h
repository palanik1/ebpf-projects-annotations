/* Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __CSUM_HELPERS_H
#define __CSUM_HELPERS_H

#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <stdbool.h>

#include "bpf.h"
#include "bpf_endian.h"
#include "bpf_helpers.h"

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 30,
  "endLine": 39,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "csum_fold_helper",
  "developer_inline_comments": [
    {
      "start_line": 1,
      "end_line": 15,
      "text": "/* Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.\n *\n * This program is free software; you can redistribute it and/or modify\n * it under the terms of the GNU General Public License as published by\n * the Free Software Foundation; version 2 of the License.\n *\n * This program is distributed in the hope that it will be useful,\n * but WITHOUT ANY WARRANTY; without even the implied warranty of\n * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n * GNU General Public License for more details.\n *\n * You should have received a copy of the GNU General Public License along\n * with this program; if not, write to the Free Software Foundation, Inc.,\n * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "__u64 csum"
  ],
  "output": "staticinline__u16",
  "helper": [],
  "compatibleHookpoints": [
    "sk_skb",
    "sched_cls",
    "raw_tracepoint",
    "cgroup_device",
    "sched_act",
    "perf_event",
    "lwt_xmit",
    "cgroup_sysctl",
    "sk_msg",
    "lwt_in",
    "cgroup_sock_addr",
    "sk_reuseport",
    "cgroup_skb",
    "lwt_out",
    "kprobe",
    "flow_dissector",
    "tracepoint",
    "socket_filter",
    "raw_tracepoint_writable",
    "sock_ops",
    "cgroup_sock",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline __u16 csum_fold_helper (__u64 csum)\n",
    "{\n",
    "    int i;\n",
    "\n",
    "#pragma unroll\n",
    "    for (i = 0; i < 4; i++) {\n",
    "        if (csum >> 16)\n",
    "            csum = (csum & 0xffff) + (csum >> 16);\n",
    "    }\n",
    "    return ~csum;\n",
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
__attribute__((__always_inline__)) static inline __u16 csum_fold_helper(
    __u64 csum) {
  int i;
#pragma unroll
  for (i = 0; i < 4; i++) {
    if (csum >> 16)
      csum = (csum & 0xffff) + (csum >> 16);
  }
  return ~csum;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 41,
  "endLine": 43,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "min_helper",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "int a",
    " int b"
  ],
  "output": "staticint",
  "helper": [],
  "compatibleHookpoints": [
    "sk_skb",
    "sched_cls",
    "raw_tracepoint",
    "cgroup_device",
    "sched_act",
    "perf_event",
    "lwt_xmit",
    "cgroup_sysctl",
    "sk_msg",
    "lwt_in",
    "cgroup_sock_addr",
    "sk_reuseport",
    "cgroup_skb",
    "lwt_out",
    "kprobe",
    "flow_dissector",
    "tracepoint",
    "socket_filter",
    "raw_tracepoint_writable",
    "sock_ops",
    "cgroup_sock",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static int min_helper (int a, int b)\n",
    "{\n",
    "    return a < b ? a : b;\n",
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
__attribute__((__always_inline__)) static int min_helper(int a, int b) {
  return a < b ? a : b;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 45,
  "endLine": 49,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "ipv4_csum",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "void *data_start",
    " int data_size",
    " __u64 *csum"
  ],
  "output": "staticinlinevoid",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline void ipv4_csum (void *data_start, int data_size, __u64 *csum)\n",
    "{\n",
    "    *csum = bpf_csum_diff (0, 0, data_start, data_size, *csum);\n",
    "    *csum = csum_fold_helper (*csum);\n",
    "}\n"
  ],
  "called_function_list": [
    "csum_fold_helper"
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
__attribute__((__always_inline__)) static inline void
ipv4_csum(void* data_start, int data_size, __u64* csum) {
  *csum = bpf_csum_diff(0, 0, data_start, data_size, *csum);
  *csum = csum_fold_helper(*csum);
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [],
  "helperCallParams": {},
  "startLine": 51,
  "endLine": 60,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "ipv4_csum_inline",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "void *iph",
    " __u64 *csum"
  ],
  "output": "staticinlinevoid",
  "helper": [],
  "compatibleHookpoints": [
    "sk_skb",
    "sched_cls",
    "raw_tracepoint",
    "cgroup_device",
    "sched_act",
    "perf_event",
    "lwt_xmit",
    "cgroup_sysctl",
    "sk_msg",
    "lwt_in",
    "cgroup_sock_addr",
    "sk_reuseport",
    "cgroup_skb",
    "lwt_out",
    "kprobe",
    "flow_dissector",
    "tracepoint",
    "socket_filter",
    "raw_tracepoint_writable",
    "sock_ops",
    "cgroup_sock",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline void ipv4_csum_inline (void *iph, __u64 *csum)\n",
    "{\n",
    "    __u16 *next_iph_u16 = (__u16 *) iph;\n",
    "\n",
    "#pragma clang loop unroll(full)\n",
    "    for (int i = 0; i < sizeof (struct iphdr) >> 1; i++) {\n",
    "        *csum += *next_iph_u16++;\n",
    "    }\n",
    "    *csum = csum_fold_helper (*csum);\n",
    "}\n"
  ],
  "called_function_list": [
    "csum_fold_helper",
    "unroll"
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
__attribute__((__always_inline__)) static inline void ipv4_csum_inline(
    void* iph,
    __u64* csum) {
  __u16* next_iph_u16 = (__u16*)iph;
#pragma clang loop unroll(full)
  for (int i = 0; i < sizeof(struct iphdr) >> 1; i++) {
    *csum += *next_iph_u16++;
  }
  *csum = csum_fold_helper(*csum);
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 62,
  "endLine": 73,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "ipv4_l4_csum",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "void *data_start",
    " int data_size",
    " __u64 *csum",
    " struct iphdr *iph"
  ],
  "output": "staticinlinevoid",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline void ipv4_l4_csum (void *data_start, int data_size, __u64 *csum, struct iphdr *iph)\n",
    "{\n",
    "    __u32 tmp = 0;\n",
    "    *csum = bpf_csum_diff (0, 0, &iph->saddr, sizeof (__be32), *csum);\n",
    "    *csum = bpf_csum_diff (0, 0, &iph->daddr, sizeof (__be32), *csum);\n",
    "    tmp = __builtin_bswap32 ((__u32) (iph -> protocol));\n",
    "    *csum = bpf_csum_diff (0, 0, &tmp, sizeof (__u32), *csum);\n",
    "    tmp = __builtin_bswap32 ((__u32) (data_size));\n",
    "    *csum = bpf_csum_diff (0, 0, &tmp, sizeof (__u32), *csum);\n",
    "    *csum = bpf_csum_diff (0, 0, data_start, data_size, *csum);\n",
    "    *csum = csum_fold_helper (*csum);\n",
    "}\n"
  ],
  "called_function_list": [
    "csum_fold_helper",
    "__builtin_bswap32"
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
__attribute__((__always_inline__)) static inline void
ipv4_l4_csum(void* data_start, int data_size, __u64* csum, struct iphdr* iph) {
  __u32 tmp = 0;
  *csum = bpf_csum_diff(0, 0, &iph->saddr, sizeof(__be32), *csum);
  *csum = bpf_csum_diff(0, 0, &iph->daddr, sizeof(__be32), *csum);
  tmp = __builtin_bswap32((__u32)(iph->protocol));
  *csum = bpf_csum_diff(0, 0, &tmp, sizeof(__u32), *csum);
  tmp = __builtin_bswap32((__u32)(data_size));
  *csum = bpf_csum_diff(0, 0, &tmp, sizeof(__u32), *csum);
  *csum = bpf_csum_diff(0, 0, data_start, data_size, *csum);
  *csum = csum_fold_helper(*csum);
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 75,
  "endLine": 88,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "ipv6_csum",
  "developer_inline_comments": [
    {
      "start_line": 77,
      "end_line": 77,
      "text": "// ipv6 pseudo header"
    },
    {
      "start_line": 85,
      "end_line": 85,
      "text": "// sum over payload"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "void *data_start",
    " int data_size",
    " __u64 *csum",
    " struct ipv6hdr *ip6h"
  ],
  "output": "staticinlinevoid",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline void ipv6_csum (void *data_start, int data_size, __u64 *csum, struct ipv6hdr *ip6h)\n",
    "{\n",
    "    __u32 tmp = 0;\n",
    "    *csum = bpf_csum_diff (0, 0, &ip6h->saddr, sizeof (struct in6_addr), *csum);\n",
    "    *csum = bpf_csum_diff (0, 0, &ip6h->daddr, sizeof (struct in6_addr), *csum);\n",
    "    tmp = __builtin_bswap32 ((__u32) (data_size));\n",
    "    *csum = bpf_csum_diff (0, 0, &tmp, sizeof (__u32), *csum);\n",
    "    tmp = __builtin_bswap32 ((__u32) (ip6h -> nexthdr));\n",
    "    *csum = bpf_csum_diff (0, 0, &tmp, sizeof (__u32), *csum);\n",
    "    *csum = bpf_csum_diff (0, 0, data_start, data_size, *csum);\n",
    "    *csum = csum_fold_helper (*csum);\n",
    "}\n"
  ],
  "called_function_list": [
    "csum_fold_helper",
    "__builtin_bswap32"
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
__attribute__((__always_inline__)) static inline void
ipv6_csum(void* data_start, int data_size, __u64* csum, struct ipv6hdr* ip6h) {
  // ipv6 pseudo header
  __u32 tmp = 0;
  *csum = bpf_csum_diff(0, 0, &ip6h->saddr, sizeof(struct in6_addr), *csum);
  *csum = bpf_csum_diff(0, 0, &ip6h->daddr, sizeof(struct in6_addr), *csum);
  tmp = __builtin_bswap32((__u32)(data_size));
  *csum = bpf_csum_diff(0, 0, &tmp, sizeof(__u32), *csum);
  tmp = __builtin_bswap32((__u32)(ip6h->nexthdr));
  *csum = bpf_csum_diff(0, 0, &tmp, sizeof(__u32), *csum);
  // sum over payload
  *csum = bpf_csum_diff(0, 0, data_start, data_size, *csum);
  *csum = csum_fold_helper(*csum);
}

#ifdef GUE_ENCAP

// Next four methods are helper methods to add or remove IP(6) pseudo header
// unto the given csum value.

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 95,
  "endLine": 127,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "add_pseudo_ipv6_header",
  "developer_inline_comments": [
    {
      "start_line": 92,
      "end_line": 92,
      "text": "// Next four methods are helper methods to add or remove IP(6) pseudo header"
    },
    {
      "start_line": 93,
      "end_line": 93,
      "text": "// unto the given csum value."
    },
    {
      "start_line": 110,
      "end_line": 110,
      "text": "// convert 16-bit payload in network order to 32-bit in network order"
    },
    {
      "start_line": 111,
      "end_line": 111,
      "text": "// e.g. payload len: 0x0102 to be written as 0x02010000 in network order"
    },
    {
      "start_line": 113,
      "end_line": 113,
      "text": "/* back to network byte order */"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv6hdr *ip6h",
    " __u64 *csum"
  ],
  "output": "staticinline__s64",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline __s64 add_pseudo_ipv6_header (struct ipv6hdr *ip6h, __u64 *csum)\n",
    "{\n",
    "    __s64 ret;\n",
    "    __u32 tmp = 0;\n",
    "    ret = bpf_csum_diff (0, 0, & ip6h -> saddr, sizeof (struct in6_addr), * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    ret = bpf_csum_diff (0, 0, & ip6h -> daddr, sizeof (struct in6_addr), * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    tmp = (__u32) bpf_ntohs (ip6h->payload_len);\n",
    "    tmp = bpf_htonl (tmp);\n",
    "    ret = bpf_csum_diff (0, 0, & tmp, sizeof (__u32), * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    tmp = __builtin_bswap32 ((__u32) (ip6h -> nexthdr));\n",
    "    ret = bpf_csum_diff (0, 0, & tmp, sizeof (__u32), * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "bpf_ntohs",
    "__builtin_bswap32",
    "bpf_htonl"
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
__attribute__((__always_inline__)) static inline __s64 add_pseudo_ipv6_header(
    struct ipv6hdr* ip6h,
    __u64* csum) {
  __s64 ret;
  __u32 tmp = 0;
  ret = bpf_csum_diff(0, 0, &ip6h->saddr, sizeof(struct in6_addr), *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  ret = bpf_csum_diff(0, 0, &ip6h->daddr, sizeof(struct in6_addr), *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  // convert 16-bit payload in network order to 32-bit in network order
  // e.g. payload len: 0x0102 to be written as 0x02010000 in network order
  tmp = (__u32)bpf_ntohs(ip6h->payload_len);
  /* back to network byte order */
  tmp = bpf_htonl(tmp);
  ret = bpf_csum_diff(0, 0, &tmp, sizeof(__u32), *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  tmp = __builtin_bswap32((__u32)(ip6h->nexthdr));
  ret = bpf_csum_diff(0, 0, &tmp, sizeof(__u32), *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  return 0;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 129,
  "endLine": 158,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "rem_pseudo_ipv6_header",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv6hdr *ip6h",
    " __u64 *csum"
  ],
  "output": "staticinline__s64",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline __s64 rem_pseudo_ipv6_header (struct ipv6hdr *ip6h, __u64 *csum)\n",
    "{\n",
    "    __s64 ret;\n",
    "    __u32 tmp = 0;\n",
    "    ret = bpf_csum_diff (& ip6h -> saddr, sizeof (struct in6_addr), 0, 0, * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    ret = bpf_csum_diff (& ip6h -> daddr, sizeof (struct in6_addr), 0, 0, * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    tmp = (__u32) bpf_ntohs (ip6h->payload_len);\n",
    "    tmp = bpf_htonl (tmp);\n",
    "    ret = bpf_csum_diff (& tmp, sizeof (__u32), 0, 0, * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    tmp = __builtin_bswap32 ((__u32) (ip6h -> nexthdr));\n",
    "    ret = bpf_csum_diff (& tmp, sizeof (__u32), 0, 0, * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "bpf_ntohs",
    "__builtin_bswap32",
    "bpf_htonl"
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
__attribute__((__always_inline__)) static inline __s64 rem_pseudo_ipv6_header(
    struct ipv6hdr* ip6h,
    __u64* csum) {
  __s64 ret;
  __u32 tmp = 0;
  ret = bpf_csum_diff(&ip6h->saddr, sizeof(struct in6_addr), 0, 0, *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  ret = bpf_csum_diff(&ip6h->daddr, sizeof(struct in6_addr), 0, 0, *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  tmp = (__u32)bpf_ntohs(ip6h->payload_len);
  tmp = bpf_htonl(tmp);
  ret = bpf_csum_diff(&tmp, sizeof(__u32), 0, 0, *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  tmp = __builtin_bswap32((__u32)(ip6h->nexthdr));
  ret = bpf_csum_diff(&tmp, sizeof(__u32), 0, 0, *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  return 0;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 160,
  "endLine": 189,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "add_pseudo_ipv4_header",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct iphdr *iph",
    " __u64 *csum"
  ],
  "output": "staticinline__s64",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline __s64 add_pseudo_ipv4_header (struct iphdr *iph, __u64 *csum)\n",
    "{\n",
    "    __s64 ret;\n",
    "    __u32 tmp = 0;\n",
    "    ret = bpf_csum_diff (0, 0, & iph -> saddr, sizeof (__be32), * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    ret = bpf_csum_diff (0, 0, & iph -> daddr, sizeof (__be32), * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    tmp = (__u32) bpf_ntohs (iph->tot_len) - sizeof (struct iphdr);\n",
    "    tmp = bpf_htonl (tmp);\n",
    "    ret = bpf_csum_diff (0, 0, & tmp, sizeof (__u32), * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    tmp = __builtin_bswap32 ((__u32) (iph -> protocol));\n",
    "    ret = bpf_csum_diff (0, 0, & tmp, sizeof (__u32), * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "bpf_ntohs",
    "__builtin_bswap32",
    "bpf_htonl"
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
__attribute__((__always_inline__)) static inline __s64 add_pseudo_ipv4_header(
    struct iphdr* iph,
    __u64* csum) {
  __s64 ret;
  __u32 tmp = 0;
  ret = bpf_csum_diff(0, 0, &iph->saddr, sizeof(__be32), *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  ret = bpf_csum_diff(0, 0, &iph->daddr, sizeof(__be32), *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  tmp = (__u32)bpf_ntohs(iph->tot_len) - sizeof(struct iphdr);
  tmp = bpf_htonl(tmp);
  ret = bpf_csum_diff(0, 0, &tmp, sizeof(__u32), *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  tmp = __builtin_bswap32((__u32)(iph->protocol));
  ret = bpf_csum_diff(0, 0, &tmp, sizeof(__u32), *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  return 0;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 191,
  "endLine": 220,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "rem_pseudo_ipv4_header",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct iphdr *iph",
    " __u64 *csum"
  ],
  "output": "staticinline__s64",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline __s64 rem_pseudo_ipv4_header (struct iphdr *iph, __u64 *csum)\n",
    "{\n",
    "    __s64 ret;\n",
    "    __u32 tmp = 0;\n",
    "    ret = bpf_csum_diff (& iph -> saddr, sizeof (__be32), 0, 0, * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    ret = bpf_csum_diff (& iph -> daddr, sizeof (__be32), 0, 0, * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    tmp = (__u32) bpf_ntohs (iph->tot_len) - sizeof (struct iphdr);\n",
    "    tmp = bpf_htonl (tmp);\n",
    "    ret = bpf_csum_diff (& tmp, sizeof (__u32), 0, 0, * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    tmp = __builtin_bswap32 ((__u32) (iph -> protocol));\n",
    "    ret = bpf_csum_diff (& tmp, sizeof (__u32), 0, 0, * csum);\n",
    "    if (ret < 0) {\n",
    "        return ret;\n",
    "    }\n",
    "    *csum = ret;\n",
    "    return 0;\n",
    "}\n"
  ],
  "called_function_list": [
    "bpf_ntohs",
    "__builtin_bswap32",
    "bpf_htonl"
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
__attribute__((__always_inline__)) static inline __s64 rem_pseudo_ipv4_header(
    struct iphdr* iph,
    __u64* csum) {
  __s64 ret;
  __u32 tmp = 0;
  ret = bpf_csum_diff(&iph->saddr, sizeof(__be32), 0, 0, *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  ret = bpf_csum_diff(&iph->daddr, sizeof(__be32), 0, 0, *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  tmp = (__u32)bpf_ntohs(iph->tot_len) - sizeof(struct iphdr);
  tmp = bpf_htonl(tmp);
  ret = bpf_csum_diff(&tmp, sizeof(__u32), 0, 0, *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  tmp = __builtin_bswap32((__u32)(iph->protocol));
  ret = bpf_csum_diff(&tmp, sizeof(__u32), 0, 0, *csum);
  if (ret < 0) {
    return ret;
  }
  *csum = ret;
  return 0;
}

/*
 * The following methods concern computation of checksum for GUE encapsulated
 * header for various combination of ip-headers.
 *
 * csum computation for the GUE header is implemented as the Eqn 3 in RFC-1624
 * https://tools.ietf.org/html/rfc1624#section-2
 * New checksum (HC') = ~(~HC + ~m + m')
 * where: HC  - old checksum in header
 *        HC' - new checksum in header
 *        m   - old value of a 16-bit field
 *        m'  - new value of a 16-bit field
 */
/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 234,
  "endLine": 268,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "gue_csum_v6",
  "developer_inline_comments": [
    {
      "start_line": 222,
      "end_line": 233,
      "text": "/*\n * The following methods concern computation of checksum for GUE encapsulated\n * header for various combination of ip-headers.\n *\n * csum computation for the GUE header is implemented as the Eqn 3 in RFC-1624\n * https://tools.ietf.org/html/rfc1624#section-2\n * New checksum (HC') = ~(~HC + ~m + m')\n * where: HC  - old checksum in header\n *        HC' - new checksum in header\n *        m   - old value of a 16-bit field\n *        m'  - new value of a 16-bit field\n */"
    },
    {
      "start_line": 241,
      "end_line": 241,
      "text": "// one's complement of csum from the original transport header"
    },
    {
      "start_line": 243,
      "end_line": 243,
      "text": "// add the original csum value from the transport header"
    }
  ],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv6hdr *outer_ip6h",
    " struct udphdr *udph",
    " struct ipv6hdr *inner_ip6h",
    " __u64 *csum_in_hdr"
  ],
  "output": "staticinlinebool",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline bool gue_csum_v6 (struct ipv6hdr *outer_ip6h, struct udphdr *udph, struct ipv6hdr *inner_ip6h, __u64 *csum_in_hdr)\n",
    "{\n",
    "    __s64 ret;\n",
    "    __u32 tmp = 0;\n",
    "    __u32 seed = (~(*csum_in_hdr)) & 0xffff;\n",
    "    __u32 orig_csum = (__u32) *csum_in_hdr;\n",
    "    ret = bpf_csum_diff (0, 0, & orig_csum, sizeof (__u32), seed);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    if (rem_pseudo_ipv6_header (inner_ip6h, csum_in_hdr) < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    ret = bpf_csum_diff (0, 0, inner_ip6h, sizeof (struct ipv6hdr), * csum_in_hdr);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    ret = bpf_csum_diff (0, 0, udph, sizeof (struct udphdr), * csum_in_hdr);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    if (add_pseudo_ipv6_header (outer_ip6h, csum_in_hdr) < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = csum_fold_helper (*csum_in_hdr);\n",
    "    return true;\n",
    "}\n"
  ],
  "called_function_list": [
    "add_pseudo_ipv6_header",
    "csum_fold_helper",
    "rem_pseudo_ipv6_header"
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
__attribute__((__always_inline__)) static inline bool gue_csum_v6(
    struct ipv6hdr* outer_ip6h,
    struct udphdr* udph,
    struct ipv6hdr* inner_ip6h,
    __u64* csum_in_hdr) {
  __s64 ret;
  __u32 tmp = 0;
  // one's complement of csum from the original transport header
  __u32 seed = (~(*csum_in_hdr)) & 0xffff;
  // add the original csum value from the transport header
  __u32 orig_csum = (__u32)*csum_in_hdr;
  ret = bpf_csum_diff(0, 0, &orig_csum, sizeof(__u32), seed);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  if (rem_pseudo_ipv6_header(inner_ip6h, csum_in_hdr) < 0) {
    return false;
  }
  ret = bpf_csum_diff(0, 0, inner_ip6h, sizeof(struct ipv6hdr), *csum_in_hdr);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  ret = bpf_csum_diff(0, 0, udph, sizeof(struct udphdr), *csum_in_hdr);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  if (add_pseudo_ipv6_header(outer_ip6h, csum_in_hdr) < 0) {
    return false;
  }
  *csum_in_hdr = csum_fold_helper(*csum_in_hdr);
  return true;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 270,
  "endLine": 302,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "gue_csum_v4",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct iphdr *outer_iph",
    " struct udphdr *udph",
    " struct iphdr *inner_iph",
    " __u64 *csum_in_hdr"
  ],
  "output": "staticinlinebool",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline bool gue_csum_v4 (struct iphdr *outer_iph, struct udphdr *udph, struct iphdr *inner_iph, __u64 *csum_in_hdr)\n",
    "{\n",
    "    __s64 ret;\n",
    "    __u32 tmp = 0;\n",
    "    __u32 seed = (~(*csum_in_hdr)) & 0xffff;\n",
    "    __u32 orig_csum = (__u32) *csum_in_hdr;\n",
    "    ret = bpf_csum_diff (0, 0, & orig_csum, sizeof (__u32), seed);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    if (rem_pseudo_ipv4_header (inner_iph, csum_in_hdr) < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    ret = bpf_csum_diff (0, 0, inner_iph, sizeof (struct iphdr), * csum_in_hdr);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    ret = bpf_csum_diff (0, 0, udph, sizeof (struct udphdr), * csum_in_hdr);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    if (add_pseudo_ipv4_header (outer_iph, csum_in_hdr) < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = csum_fold_helper (*csum_in_hdr);\n",
    "    return true;\n",
    "}\n"
  ],
  "called_function_list": [
    "add_pseudo_ipv4_header",
    "csum_fold_helper",
    "rem_pseudo_ipv4_header"
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
__attribute__((__always_inline__)) static inline bool gue_csum_v4(
    struct iphdr* outer_iph,
    struct udphdr* udph,
    struct iphdr* inner_iph,
    __u64* csum_in_hdr) {
  __s64 ret;
  __u32 tmp = 0;
  __u32 seed = (~(*csum_in_hdr)) & 0xffff;
  __u32 orig_csum = (__u32)*csum_in_hdr;
  ret = bpf_csum_diff(0, 0, &orig_csum, sizeof(__u32), seed);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  if (rem_pseudo_ipv4_header(inner_iph, csum_in_hdr) < 0) {
    return false;
  }
  ret = bpf_csum_diff(0, 0, inner_iph, sizeof(struct iphdr), *csum_in_hdr);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  ret = bpf_csum_diff(0, 0, udph, sizeof(struct udphdr), *csum_in_hdr);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  if (add_pseudo_ipv4_header(outer_iph, csum_in_hdr) < 0) {
    return false;
  }
  *csum_in_hdr = csum_fold_helper(*csum_in_hdr);
  return true;
}

/* 
 OPENED COMMENT BEGIN 
{
  "capabilities": [
    {
      "capability": "read_skb",
      "read_skb": [
        {
          "Project": "libbpf",
          "Return Type": "s64",
          "Description": "Compute a checksum difference , <[ from ]>(IP: 0) the raw buffer pointed by <[ from ]>(IP: 0) , of length <[ from_size ]>(IP: 1) (that must be a multiple of 4) , towards the raw buffer pointed by <[ to ]>(IP: 2) , of size <[ to_size ]>(IP: 3) (same remark). An optional <[ seed ]>(IP: 4) can be added <[ to ]>(IP: 2) the value (this can be cascaded , the <[ seed ]>(IP: 4) may come <[ from ]>(IP: 0) a previous call <[ to ]>(IP: 2) the helper). This is flexible enough <[ to ]>(IP: 2) be used in several ways: \u00b7 With <[ from_size ]>(IP: 1) == 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when pushing new data. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) == 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) checksum , it can be used when removing data <[ from ]>(IP: 0) a packet. \u00b7 With <[ from_size ]>(IP: 1) > 0 , <[ to_size ]>(IP: 3) > 0 and <[ seed ]>(IP: 4) set <[ to ]>(IP: 2) 0 , it can be used <[ to ]>(IP: 2) compute a diff. Note that <[ from_size ]>(IP: 1) and <[ to_size ]>(IP: 3) do not need <[ to ]>(IP: 2) be equal. This helper can be used in combination with bpf_l3_csum_replace() and bpf_l4_csum_replace() , <[ to ]>(IP: 2) which one can feed in the difference computed with bpf_csum_diff(). ",
          "Return": " The checksum result, or a negative error code in case of failure.",
          "Function Name": "bpf_csum_diff",
          "Input Params": [
            "{Type: __be32 ,Var: *from}",
            "{Type:  u32 ,Var: from_size}",
            "{Type:  __be32 ,Var: *to}",
            "{Type:  u32 ,Var: to_size}",
            "{Type:  __wsum ,Var: seed}"
          ],
          "compatible_hookpoints": [
            "sched_cls",
            "sched_act",
            "xdp",
            "lwt_in",
            "lwt_out",
            "lwt_xmit",
            "lwt_seg6local"
          ],
          "capabilities": [
            "read_skb"
          ]
        }
      ]
    }
  ],
  "helperCallParams": {},
  "startLine": 304,
  "endLine": 336,
  "File": "/home/sayandes/ebpf-projects-annotations/examples/katran/csum_helpers.h",
  "funcName": "gue_csum_v4_in_v6",
  "developer_inline_comments": [],
  "updateMaps": [],
  "readMaps": [],
  "input": [
    "struct ipv6hdr *outer_ip6h",
    " struct udphdr *udph",
    " struct iphdr *inner_iph",
    " __u64 *csum_in_hdr"
  ],
  "output": "staticinlinebool",
  "helper": [
    "bpf_csum_diff"
  ],
  "compatibleHookpoints": [
    "lwt_xmit",
    "sched_cls",
    "lwt_in",
    "lwt_out",
    "sched_act",
    "xdp",
    "lwt_seg6local"
  ],
  "source": [
    "static inline bool gue_csum_v4_in_v6 (struct ipv6hdr *outer_ip6h, struct udphdr *udph, struct iphdr *inner_iph, __u64 *csum_in_hdr)\n",
    "{\n",
    "    __s64 ret;\n",
    "    __u32 tmp = 0;\n",
    "    __u32 seed = (~(*csum_in_hdr)) & 0xffff;\n",
    "    __u32 orig_csum = (__u32) *csum_in_hdr;\n",
    "    ret = bpf_csum_diff (0, 0, & orig_csum, sizeof (__u32), seed);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    if (rem_pseudo_ipv4_header (inner_iph, csum_in_hdr) < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    ret = bpf_csum_diff (0, 0, inner_iph, sizeof (struct iphdr), * csum_in_hdr);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    ret = bpf_csum_diff (0, 0, udph, sizeof (struct udphdr), * csum_in_hdr);\n",
    "    if (ret < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = ret;\n",
    "    if (add_pseudo_ipv6_header (outer_ip6h, csum_in_hdr) < 0) {\n",
    "        return false;\n",
    "    }\n",
    "    *csum_in_hdr = csum_fold_helper (*csum_in_hdr);\n",
    "    return true;\n",
    "}\n"
  ],
  "called_function_list": [
    "add_pseudo_ipv6_header",
    "csum_fold_helper",
    "rem_pseudo_ipv4_header"
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
__attribute__((__always_inline__)) static inline bool gue_csum_v4_in_v6(
    struct ipv6hdr* outer_ip6h,
    struct udphdr* udph,
    struct iphdr* inner_iph,
    __u64* csum_in_hdr) {
  __s64 ret;
  __u32 tmp = 0;
  __u32 seed = (~(*csum_in_hdr)) & 0xffff;
  __u32 orig_csum = (__u32)*csum_in_hdr;
  ret = bpf_csum_diff(0, 0, &orig_csum, sizeof(__u32), seed);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  if (rem_pseudo_ipv4_header(inner_iph, csum_in_hdr) < 0) {
    return false;
  }
  ret = bpf_csum_diff(0, 0, inner_iph, sizeof(struct iphdr), *csum_in_hdr);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  ret = bpf_csum_diff(0, 0, udph, sizeof(struct udphdr), *csum_in_hdr);
  if (ret < 0) {
    return false;
  }
  *csum_in_hdr = ret;
  if (add_pseudo_ipv6_header(outer_ip6h, csum_in_hdr) < 0) {
    return false;
  }
  *csum_in_hdr = csum_fold_helper(*csum_in_hdr);
  return true;
}
#endif // of GUE_ENCAP

#endif // of __CSUM_HELPERS_H