/* LLVM plugin for gcc
   Copyright (C) 2009 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not see
<http://www.gnu.org/licenses/>.  */

// LLVM headers
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/TargetFolder.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetMachineRegistry.h"

// System headers
#include <iostream>
#include <vector>

// GCC headers
#undef VISIBILITY_HIDDEN
extern "C" {
#include "intl.h"
}
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "coretypes.h"
#include "target.h"
#include "flags.h"
#include "toplev.h"
#include "tree.h"
#include "tree-flow.h"
#include "tree-pass.h"

// Plugin headers
//#include "llvm-internal.h"

using namespace llvm;

// This plugin's code is licensed under the GPLv2.  The LLVM libraries use
// the GPL compatible University of Illinois/NCSA Open Source License.
int plugin_is_GPL_compatible; // This plugin is GPL compatible.


// Global state for emitting LLVM IR.
TargetFolder *TheFolder = 0;
Module *TheModule = 0;
TargetMachine *TheTarget = 0;
//FIXMETypeConverter *TheTypeConverter = 0;

/// LazilyConfigureLLVM - Set LLVM configuration options, if not already set.
/// already created.
static void LazilyConfigureLLVM(void) {
  static bool Configured = false;
  if (Configured)
    return;

  // Initialize LLVM command line options.
  std::vector<const char*> Args;
  Args.push_back(progname); // program name

//TODO  // Allow targets to specify PIC options and other stuff to the corresponding
//TODO  // LLVM backends.
//TODO#ifdef LLVM_SET_RED_ZONE_FLAG
//TODO  LLVM_SET_RED_ZONE_FLAG(flag_disable_red_zone)
//TODO#endif
//TODO#ifdef LLVM_SET_TARGET_OPTIONS
//TODO  LLVM_SET_TARGET_OPTIONS(Args);
//TODO#endif
//TODO#ifdef LLVM_SET_MACHINE_OPTIONS
//TODO  LLVM_SET_MACHINE_OPTIONS(Args);
//TODO#endif
//TODO#ifdef LLVM_SET_IMPLICIT_FLOAT
//TODO  LLVM_SET_IMPLICIT_FLOAT(flag_no_implicit_float)
//TODO#endif

  if (time_report)
    Args.push_back("--time-passes");
  if (fast_math_flags_set_p())
    Args.push_back("--enable-unsafe-fp-math");
  if (!flag_omit_frame_pointer)
    Args.push_back("--disable-fp-elim");
  if (!flag_zero_initialized_in_bss)
    Args.push_back("--nozero-initialized-in-bss");
  if (flag_debug_asm)
    Args.push_back("--asm-verbose");
//TODO  if (flag_debug_pass_structure)
//TODO    Args.push_back("--debug-pass=Structure");
//TODO  if (flag_debug_pass_arguments)
//TODO    Args.push_back("--debug-pass=Arguments");
  if (optimize_size || optimize < 3)
    // Reduce inline limit. Default limit is 200.
    Args.push_back("--inline-threshold=50");
  if (flag_unwind_tables)
    Args.push_back("--unwind-tables");

//TODO  // If there are options that should be passed through to the LLVM backend
//TODO  // directly from the command line, do so now.  This is mainly for debugging
//TODO  // purposes, and shouldn't really be for general use.
//TODO  std::vector<std::string> ArgStrings;
//TODO
//TODO  if (flag_limited_precision > 0) {
//TODO    std::string Arg("--limit-float-precision="+utostr(flag_limited_precision));
//TODO    ArgStrings.push_back(Arg);
//TODO  }
//TODO
//TODO  if (flag_stack_protect > 0) {
//TODO    std::string Arg("--stack-protector-buffer-size=" +
//TODO                    utostr(PARAM_VALUE(PARAM_SSP_BUFFER_SIZE)));
//TODO    ArgStrings.push_back(Arg);
//TODO  }
//TODO
//TODO  for (unsigned i = 0, e = ArgStrings.size(); i != e; ++i)
//TODO    Args.push_back(ArgStrings[i].c_str());
//TODO
//TODO  std::vector<std::string> LLVM_Optns; // Avoid deallocation before opts parsed!
//TODO  if (llvm_optns) {
//TODO    SplitString(llvm_optns, LLVM_Optns);
//TODO    for(unsigned i = 0, e = LLVM_Optns.size(); i != e; ++i)
//TODO      Args.push_back(LLVM_Optns[i].c_str());
//TODO  }

  Args.push_back(0);  // Null terminator.
  int pseudo_argc = Args.size()-1;
  llvm::cl::ParseCommandLineOptions(pseudo_argc, (char**)&Args[0]);

  Configured = true;
}

/// LazilyInitializeModule - Create a module to output LLVM IR to, if it wasn't
/// already created.
static void LazilyInitializeModule(void) {
  static bool Initialized = false;
  if (Initialized)
    return;

  LazilyConfigureLLVM;

  TheModule = new Module("", getGlobalContext());

  // If the target wants to override the architecture, e.g. turning
  // powerpc-darwin-... into powerpc64-darwin-... when -m64 is enabled, do so
  // now.
  std::string TargetTriple = "x86_64-linux-gnu"; // FIXME!
//TODO  std::string TargetTriple = TARGET_NAME;
//TODO#ifdef LLVM_OVERRIDE_TARGET_ARCH
//TODO  std::string Arch = LLVM_OVERRIDE_TARGET_ARCH();
//TODO  if (!Arch.empty()) {
//TODO    std::string::size_type DashPos = TargetTriple.find('-');
//TODO    if (DashPos != std::string::npos)// If we have a sane t-t, replace the arch.
//TODO      TargetTriple = Arch + TargetTriple.substr(DashPos);
//TODO  }
//TODO#endif
//TODO#ifdef LLVM_OVERRIDE_TARGET_VERSION
//TODO  char *NewTriple;
//TODO  bool OverRidden = LLVM_OVERRIDE_TARGET_VERSION(TargetTriple.c_str(),
//TODO                                                 &NewTriple);
//TODO  if (OverRidden)
//TODO    TargetTriple = std::string(NewTriple);
//TODO#endif
  TheModule->setTargetTriple(TargetTriple);

//FIXME  TheTypeConverter = new TypeConverter();

  // Create the TargetMachine we will be generating code with.
  // FIXME: Figure out how to select the target and pass down subtarget info.
  std::string Err;
  const TargetMachineRegistry::entry *TME =
    TargetMachineRegistry::getClosestStaticTargetForModule(*TheModule, Err);
  if (!TME) {
    cerr << "Did not get a target machine! Triplet is " << TargetTriple << '\n';
    exit(1);
  }

  // Figure out the subtarget feature string we pass to the target.
  std::string FeatureStr;
//TODO  // The target can set LLVM_SET_SUBTARGET_FEATURES to configure the LLVM
//TODO  // backend.
//TODO#ifdef LLVM_SET_SUBTARGET_FEATURES
//TODO  SubtargetFeatures Features;
//TODO  LLVM_SET_SUBTARGET_FEATURES(Features);
//TODO  FeatureStr = Features.getString();
//TODO#endif
  TheTarget = TME->CtorFn(*TheModule, FeatureStr);
  assert(TheTarget->getTargetData()->isBigEndian() == BYTES_BIG_ENDIAN);

  TheFolder = new TargetFolder(TheTarget->getTargetData(), getGlobalContext());

  // Install information about target datalayout stuff into the module for
  // optimizer use.
  TheModule->setDataLayout(TheTarget->getTargetData()->
                           getStringRepresentation());

//TODO  if (optimize)
//TODO    RegisterRegAlloc::setDefault(createLinearScanRegisterAllocator);
//TODO  else
//TODO    RegisterRegAlloc::setDefault(createLocalRegisterAllocator);

//TODO  // FIXME - Do not disable debug info while writing pch.
//TODO  if (!flag_pch_file &&
//TODO      debug_info_level > DINFO_LEVEL_NONE)
//TODO    TheDebugInfo = new DebugInfo(TheModule);
//TODO}
//TODO
//TODO/// Set backend options that may only be known at codegen time.
//TODOvoid performLateBackendInitialization(void) {
//TODO  // The Ada front-end sets flag_exceptions only after processing the file.
//TODO  ExceptionHandling = flag_exceptions;
//TODO  for (Module::iterator I = TheModule->begin(), E = TheModule->end();
//TODO       I != E; ++I)
//TODO    if (!I->isDeclaration()) {
//TODO      if (flag_disable_red_zone)
//TODO        I->addFnAttr(Attribute::NoRedZone);
//TODO      if (flag_no_implicit_float)
//TODO        I->addFnAttr(Attribute::NoImplicitFloat);
//TODO    }
//TODO}
//TODO
//TODOvoid llvm_lang_dependent_init(const char *Name) {
//TODO  if (TheDebugInfo)
//TODO    TheDebugInfo->Initialize();
//TODO  if (Name)
//TODO    TheModule->setModuleIdentifier(Name);
//TODO}
  Initialized = true;
}

/// execute_emit_llvm - Turn a gimple function into LLVM IR.
static unsigned int
execute_emit_llvm (void)
{
//TODO Don't want to use sorry at this stage...
//TODO  if (cfun->nonlocal_goto_save_area)
//TODO    sorry("%Jnon-local gotos not supported by LLVM", fndecl);

//TODO Do we want to do this?  Will the warning set sorry_count etc?
//TODO    enum symbol_visibility vis = DECL_VISIBILITY (current_function_decl);
//TODO
//TODO    if (vis != VISIBILITY_DEFAULT)
//TODO      // "asm_out.visibility" emits an important warning if we're using a
//TODO      // visibility that's not supported by the target.
//TODO      targetm.asm_out.visibility(current_function_decl, vis);

  // There's no need to defer outputting this function any more; we
  // know we want to output it.
  DECL_DEFER_OUTPUT(current_function_decl) = 0;

  // Convert the AST to raw/ugly LLVM code.
//FIXME  TreeToLLVM Emitter(current_function_decl);
cout << "Yo!\n";

//TODO#if 0
//TODO  if (dump_file) {
//TODO    fprintf (dump_file,
//TODO             "\n\n;;\n;; Full LLVM generated for this function:\n;;\n");
//TODO    Fn->dump();
//TODO  }
//TODO#endif
//TODO
//TODO  performLateBackendInitialization();
//TODO  createPerFunctionOptimizationPasses();
//TODO
//TODO  if (PerFunctionPasses)
//TODO    PerFunctionPasses->run(*Fn);
//TODO
//TODO  // TODO: Nuke the .ll code for the function at -O[01] if we don't want to
//TODO  // inline it or something else.

  // Finally, we have written out this function!
  TREE_ASM_WRITTEN(current_function_decl) = 1;

  execute_free_datastructures ();
  return 0;
}

/// pass_emit_llvm - RTL pass that turns gimple functions into LLVM IR.
static struct rtl_opt_pass pass_emit_llvm =
{
    {
      RTL_PASS,
      "emit_llvm",				/* name */
      NULL,					/* gate */
      execute_emit_llvm,			/* execute */
      NULL,					/* sub */
      NULL,					/* next */
      0,					/* static_pass_number */
      TV_EXPAND,				/* tv_id */
      PROP_gimple_lcf | PROP_gimple_leh |
	PROP_gimple_lomp | PROP_cfg,		/* properties_required */
      0,					/* properties_provided */
      PROP_ssa | PROP_trees,			/* properties_destroyed */
      TODO_dump_func | TODO_verify_flow
        | TODO_verify_stmts,			/* todo_flags_start */
      TODO_ggc_collect				/* todo_flags_finish */
    }
};


/// gate_null - Gate method for a pass that does nothing.
static bool
gate_null (void)
{
  return false;
}


/// pass_gimple_null - Gimple pass that does nothing.
static struct gimple_opt_pass pass_gimple_null =
{
    {
      GIMPLE_PASS,
      NULL,					/* name */
      gate_null,				/* gate */
      NULL,					/* execute */
      NULL,					/* sub */
      NULL,					/* next */
      0,					/* static_pass_number */
      TV_NONE,					/* tv_id */
      0,					/* properties_required */
      0,					/* properties_provided */
      0,					/* properties_destroyed */
      0,					/* todo_flags_start */
      0						/* todo_flags_finish */
    }
};

/// pass_rtl_null - RTL pass that does nothing.
static struct rtl_opt_pass pass_rtl_null =
{
    {
      RTL_PASS,
      NULL,					/* name */
      gate_null,				/* gate */
      NULL,					/* execute */
      NULL,					/* sub */
      NULL,					/* next */
      0,					/* static_pass_number */
      TV_NONE,					/* tv_id */
      0,					/* properties_required */
      0,					/* properties_provided */
      0,					/* properties_destroyed */
      0,					/* todo_flags_start */
      0						/* todo_flags_finish */
    }
};

/// plugin_init - The initialization routine called by GCC.  Defined in
/// gcc-plugin.h.
int plugin_init (struct plugin_name_args *plugin_info,
                 struct plugin_gcc_version *version) {
  const char *plugin_name = plugin_info->base_name;
  bool disable_gcc_optimizations = true;
  struct plugin_pass pass_info;

  // Check that the running gcc is the same as the gcc we were built against.
  // If not, refuse to load.  This seems wise when developing against a fast
  // moving gcc tree.
  // TODO: Make the check milder if doing a "release build".
  if (!plugin_default_version_check (version, &gcc_version)) {
    // TODO: calling a gcc routine when there is a version mismatch is
    // dangerous.  On the other hand, failing without an explanation is
    // obscure.  Could send a message to std::cerr instead, but bypassing
    // the gcc error reporting and translating mechanism is kind of sucky.
    error(G_("plugin %qs: gcc version mismatch"), plugin_name);
    return 1;
  }

  // Process any plugin arguments.
  {
    int argc = plugin_info->argc;
    struct plugin_argument *argv = plugin_info->argv;

    for (int i = 0; i < argc; ++i) {
      if (!strcmp (argv[i].key, "enable-gcc-optzns")) {
        if (argv[i].value)
          warning (0, G_("option '-fplugin-arg-%s-%s=%s' ignored"
                         " (superfluous '=%s')"),
                   plugin_name, argv[i].key, argv[i].value, argv[i].value);
        else
          disable_gcc_optimizations = false;
      } else {
        warning (0, G_("plugin %qs: unrecognized argument %qs ignored"),
                 plugin_name, argv[i].key);
      }
    }
  }

  // Replace rtl expansion with gimple to LLVM conversion.
  pass_info.pass = &pass_emit_llvm.pass;
  pass_info.reference_pass_name = "expand";
  pass_info.ref_pass_instance_number = 0;
  pass_info.pos_op = PASS_POS_REPLACE;
  register_callback (plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

  // Turn off all rtl passes.
  pass_info.pass = &pass_gimple_null.pass;
  pass_info.reference_pass_name = "*rest_of_compilation";
  pass_info.ref_pass_instance_number = 0;
  pass_info.pos_op = PASS_POS_REPLACE;
  register_callback (plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

  pass_info.pass = &pass_rtl_null.pass;
  pass_info.reference_pass_name = "*clean_state";
  pass_info.ref_pass_instance_number = 0;
  pass_info.pos_op = PASS_POS_REPLACE;
  register_callback (plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

  if (disable_gcc_optimizations) {
    // Turn off all gcc optimization passes.
    // TODO: figure out a good way of turning off ipa passes.
    pass_info.pass = &pass_gimple_null.pass;
    pass_info.reference_pass_name = "*all_optimizations";
    pass_info.ref_pass_instance_number = 0;
    pass_info.pos_op = PASS_POS_REPLACE;
    register_callback (plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
  }

  return 0;
}
