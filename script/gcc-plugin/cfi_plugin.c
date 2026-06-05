/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2024-2026 [Tera Naam]
 * cfi_plugin.c - Control Flow Integrity plugin for Linuxab OS
 *
 * Validates indirect call targets to prevent ROP/JOP attacks.
 * Adds type-based checking for function pointers.
 *
 * Linuxab OS - The Future of Computing
 */

#include "gcc-common.h"

/* Plugin metadata */
int plugin_is_GPL_compatible;

static struct plugin_info cfi_plugin_info = {
	.version = LINUXAB_GCC_PLUGIN_VERSION,
	.help = "Control Flow Integrity for indirect calls",
};

/* CFI mode */
enum cfi_mode {
	CFI_NONE = 0,
	CFI_MONOTONIC,      /* Basic: all valid targets */
	CFI_FINE_GRAINED,   /* Type-based checking */
	CFI_SHADOW_STACK,   /* With shadow stack */
};

static enum cfi_mode cfi_mode = CFI_FINE_GRAINED;

/* Shadow stack operations */
static GTY(()) tree cfi_check_fn;
static GTY(()) tree cfi_shadow_push_fn;
static GTY(()) tree cfi_shadow_pop_fn;

/* Type hash for fine-grained CFI */
static unsigned int hash_type(tree type)
{
	unsigned int hash = 0;
	tree arg;

	/* Hash return type */
	hash ^= TYPE_HASH(TREE_TYPE(type));

	/* Hash argument types */
	for (arg = TYPE_ARG_TYPES(type); arg; arg = TREE_CHAIN(arg)) {
		hash ^= TYPE_HASH(TREE_VALUE(arg));
		hash = (hash << 5) | (hash >> 27);
	}

	return hash;
}

/* Insert CFI check before indirect call */
static void insert_cfi_check(gimple_stmt_iterator *gsi, gcall *call)
{
	tree fn = gimple_call_fn(call);
	tree type = TREE_TYPE(TREE_TYPE(fn));
	unsigned int type_hash = hash_type(type);
	gassign *hash_stmt;
	gcond *check_stmt;
	basic_block bb, then_bb, else_bb;
	edge e;

	/* Build: if (cfi_check(target, hash)) call; else abort(); */

	/* Insert hash check */
	hash_stmt = gimple_build_assign(
		gimple_build_call(cfi_check_fn, 2, fn,
				  build_int_cst(unsigned_type_node, type_hash)),
		NULL_TREE);

	gimple_set_location(hash_stmt, gimple_location(call));
	gsi_insert_before(gsi, hash_stmt, GSI_SAME_STMT);

	/* Build conditional */
	check_stmt = gimple_build_cond(NE_EXPR, gimple_assign_lhs(hash_stmt),
				       boolean_false_node, NULL_TREE, NULL_TREE);
	gimple_set_location(check_stmt, gimple_location(call));
	gsi_insert_before(gsi, check_stmt, GSI_SAME_STMT);

	/* Split block */
	e = split_block(gsi_bb(*gsi), gsi_stmt(*gsi));
	bb = e->src;
	then_bb = e->dest;
	else_bb = create_empty_bb(bb);

	/* Redirect edge */
	e->flags &= ~EDGE_FALLTHRU;
	e->flags |= EDGE_TRUE_VALUE;
	make_edge(bb, else_bb, EDGE_FALSE_VALUE);

	/* Insert abort in else block */
	gcall *abort_call = gimple_build_call(
		lookup_name(get_identifier("__cfi_abort")), 0);
	gimple_seq abort_seq = NULL;
	gimple_seq_add_stmt(&abort_seq, abort_call);
	set_bb_seq(else_bb, abort_seq);

	/* Merge then block back */
	make_single_succ_edge(then_bb, else_bb, EDGE_FALLTHRU);
}

/* Main pass */
static unsigned int execute_cfi(void)
{
	basic_block bb;
	gimple_stmt_iterator gsi;

	FOR_EACH_BB_FN(bb, cfun) {
		for (gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
			gimple *stmt = gsi_stmt(gsi);

			if (gimple_code(stmt) == GIMPLE_CALL) {
				gcall *call = as_a<<gcall *>(stmt);
				tree fn = gimple_call_fn(call);

				/* Check if indirect call */
				if (TREE_CODE(fn) == SSA_NAME &&
				    TREE_CODE(TREE_TYPE(fn)) == POINTER_TYPE &&
				    TREE_CODE(TREE_TYPE(TREE_TYPE(fn))) == FUNCTION_TYPE) {
					insert_cfi_check(&gsi, call);
				}
			}
		}
	}

	return 0;
}

/* Pass definition */
static struct gimple_opt_pass pass_cfi = {
	.pass = {
		.type = GIMPLE_PASS,
		.name = "cfi",
		.optinfo_flags = OPTGROUP_NONE,
		.tv_id = TV_NONE,
		.properties_required = PROP_gimple_any,
		.properties_provided = 0,
		.properties_destroyed = 0,
		.todo_flags_start = 0,
		.todo_flags_finish = TODO_verify_stmts,
		.execute = execute_cfi,
	},
};

/* Plugin initialization */
int plugin_init(struct plugin_name_args *info,
		struct plugin_gcc_version *ver)
{
	const char *plugin_name = info->base_name;
	int argc = info->argc;
	struct plugin_argument *argv = info->argv;
	int i;

	if (!plugin_default_version_check(ver, &gcc_version)) {
		error("CFI plugin incompatible with GCC %s", ver->basever);
		return 1;
	}

	/* Parse mode */
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i].key, "mode") == 0) {
			if (strcmp(argv[i].value, "fine") == 0)
				cfi_mode = CFI_FINE_GRAINED;
			else if (strcmp(argv[i].value, "shadow") == 0)
				cfi_mode = CFI_SHADOW_STACK;
		}
	}

	register_callback(plugin_name, PLUGIN_INFO, NULL, &cfi_plugin_info);

	/* Register pass before expansion */
	struct register_pass_info pass_info = {
		.pass = &pass_cfi.pass,
		.reference_pass_name = "optimized",
		.ref_pass_instance_number = 1,
		.pos_op = PASS_POS_INSERT_BEFORE,
	};

	register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP,
			  NULL, &pass_info);

	printf("Linuxab CFI plugin loaded (mode=%d)\n", cfi_mode);

	return 0;
}
