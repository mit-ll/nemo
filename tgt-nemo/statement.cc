#include "ttb.h"
#include "ttb_signal.h"

#include <cassert>
#include <limits>
/*
 * Check to see if the given number (expression) can be represented
 * accurately in a long value.
 */
static bool number_is_long(ivl_expr_t expr) {
   ivl_expr_type_t type = ivl_expr_type(expr);

   assert(type == IVL_EX_NUMBER || type == IVL_EX_ULONG);

   // Make sure the ULONG can be represented correctly in a long.
   if (type == IVL_EX_ULONG) {
      unsigned long val = ivl_expr_uvalue(expr);
      if (val > static_cast<unsigned>(std::numeric_limits<long>::max())) {
         return false;
      }
      return true;
   }

   // Check to see if the number actually fits in a long.
   unsigned nbits = ivl_expr_width(expr);
   if (nbits >= 8*sizeof(long)) {
      const char*bits = ivl_expr_bits(expr);
      char pad_bit = bits[nbits-1];
      for (unsigned idx = 8*sizeof(long); idx < nbits; idx++) {
         if (bits[idx] != pad_bit) return false;
      }
   }
   return true;
}

/*
 * Return the given number (expression) as a signed long value.
 *
 * Make sure to call number_is_long() first to verify that the number
 * can be represented accurately in a long value.
 */
static long get_number_as_long(ivl_expr_t expr) {
   long imm = 0;
   switch (ivl_expr_type(expr)) {
   case IVL_EX_ULONG:
      imm = ivl_expr_uvalue(expr);
      break;

   case IVL_EX_NUMBER: {
      const char*bits = ivl_expr_bits(expr);
      unsigned nbits = ivl_expr_width(expr);
      if (nbits > 8*sizeof(long)) nbits = 8*sizeof(long);
      for (unsigned idx = 0; idx < nbits; idx++) {
         switch (bits[idx]) {
         case '0':
            break;
         case '1':
            imm |= 1L << idx;
            break;
         default:
            assert(0);
         }

         if (ivl_expr_signed(expr) && bits[nbits-1] == '1' &&
             nbits < 8*sizeof(long)) imm |= -1L << nbits;
      }
      break;
   }

   default:
      assert(0);
   }
   return imm;
}

static void process_rexpr(const TTB_Signal& lsig, ivl_expr_t rexpr, std::vector<connection_t>& connections) {
  const ivl_expr_type_t expr_type = ivl_expr_type(rexpr);
  TTB_Signal rsig;

  switch (expr_type) {

    case IVL_EX_NUMBER:
    case IVL_EX_NONE:
      break;

    case IVL_EX_CONCAT:
      for (unsigned i = 0; i < ivl_expr_repeat(rexpr); i++) {
        for (unsigned j = 0; j < ivl_expr_parms(rexpr); j++) {
          process_rexpr(lsig, ivl_expr_parm(rexpr, j), connections);
        }
      }
      break;


    case IVL_EX_UNARY:
      process_rexpr(lsig, ivl_expr_oper1(rexpr), connections);
      break;

    case IVL_EX_BINARY:
      // @TODO: Support shifts of non-constants
      if (ivl_expr_opcode(rexpr) == 'l' || ivl_expr_opcode(rexpr) == 'r') {
        if (ivl_expr_type(ivl_expr_oper2(rexpr)) != IVL_EX_NUMBER) {
          fprintf(stderr, "WARNING: Shift by non-constant not yet supported\n");
          fprintf(stderr, "File: %s Line: %d\n", ivl_expr_file(rexpr), ivl_expr_lineno(rexpr));
          break;
        }
      }
      process_rexpr(lsig, ivl_expr_oper1(rexpr), connections);
      process_rexpr(lsig, ivl_expr_oper2(rexpr), connections);
      break;

    case IVL_EX_TERNARY:
      process_rexpr(lsig, ivl_expr_oper1(rexpr), connections);
      process_rexpr(lsig, ivl_expr_oper2(rexpr), connections);
      process_rexpr(lsig, ivl_expr_oper3(rexpr), connections);
      break;

    case IVL_EX_SELECT:
      {
        ivl_expr_t index = ivl_expr_oper2(rexpr);
        ivl_expr_t base  = ivl_expr_oper1(rexpr);
        if (ivl_expr_type(base) != IVL_EX_SIGNAL) {
          fprintf(stderr, "Base of non-signal type %d not supported\n", ivl_expr_type(base));
          fprintf(stderr, "File: %s Line: %d\n", ivl_expr_file(rexpr), ivl_expr_lineno(rexpr));
          break;
        }
        rsig = ivl_expr_signal(base);
        ivl_expr_type_t index_type = ivl_expr_type(index);
        if (index_type == IVL_EX_SIGNAL) {
          //@TODO: Deal with sig[i] (where i is another signal)
          //       I believe we just have to assume it could be sig[anything]...
          //       Maybe not support for now...
          fprintf(stderr, "ERROR: Slicing with a signal not yet supported\n");
          fprintf(stderr, "File: %s Line: %d\n", ivl_expr_file(rexpr), ivl_expr_lineno(rexpr));
        } else if (index_type == IVL_EX_NUMBER) {
          if (!number_is_long(index)) {
            //@TODO: More verbose line number
            fprintf(stderr, "ERROR: Number to large\n");
            fprintf(stderr, "File: %s Line: %d\n", ivl_expr_file(rexpr), ivl_expr_lineno(rexpr));
          }
          rsig.set_lsb(get_number_as_long(index));
          rsig.set_msb(rsig.get_lsb() + ivl_expr_width(rexpr) - 1);
        } else if (index_type == IVL_EX_NONE){
          // Defaults fine
        } else {
          fprintf(stderr, "ERROR: Support of non-number type %d not supported\n", index_type);
          fprintf(stderr, "File: %s Line: %d\n", ivl_expr_file(rexpr), ivl_expr_lineno(rexpr));
          break;
        }
        print_connection(lsig, rsig, connections);
      }
      break;


    case IVL_EX_SIGNAL: 
      rsig = ivl_expr_signal(rexpr);
      print_connection(lsig, rsig, connections);
      break;

    default:
      fprintf(stderr, "ERROR: Unsupported expression %d\n", expr_type);
      fprintf(stderr, "File: %s Line: %d\n", ivl_expr_file(rexpr), ivl_expr_lineno(rexpr));
      break;
  }

}

static void process_lvals(ivl_statement_t stmt, sig_map_t& ffs, std::vector<connection_t>& connections, bool is_ff) {
  TTB_Signal sig;
  //@TODO: Deal with concated lvals (we can be more precise than this)
  for (unsigned int i = 0; i < ivl_stmt_lvals(stmt); i++) {
    ivl_lval_t lval = ivl_stmt_lval(stmt, i);
    sig = ivl_lval_sig(lval);

    //@TODO: Deal with nested lvals
    if (ivl_lval_nest(lval)) {
      fprintf(stderr, "ERROR: Cannot deal with nested lvals yet\n");
      fprintf(stderr, "File: %s Line: %d\n", ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
      return;
    }

    ivl_expr_t part;
    if ((part = ivl_lval_part_off(lval))) {
      ivl_expr_type_t expr_type = ivl_expr_type(part);
      if (expr_type == IVL_EX_SIGNAL) {
        //@TODO: Deal with sig[i] (where i is another signal)
        fprintf(stderr, "ERROR: Slicing with a signal not yet supported\n");
        fprintf(stderr, "File: %s Line: %d\n", ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
      } else if (expr_type == IVL_EX_NUMBER) {
        if (!number_is_long(part)) {
          fprintf(stderr, "ERROR: Number to large\n");
          fprintf(stderr, "File: %s Line: %d\n", ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
        }
        sig.set_lsb(get_number_as_long(part));
        sig.set_msb((sig.get_lsb() + ivl_lval_width(lval)) - 1);
      } else {
        fprintf(stderr, "ERROR: Support of non-number type index not supported\n");
        fprintf(stderr, "File: %s Line: %d\n", ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
      }
    }

    if (is_ff) {
      assert(ffs.find(sig.name()) != ffs.end());
      ffs[sig.name()]->set_isff();
    }
    process_rexpr(sig, ivl_stmt_rval(stmt), connections);
  }
}


static int process_statement(ivl_statement_t stmt, sig_map_t& ffs, std::vector<connection_t>& connections, bool is_ff) {
  const ivl_statement_type_t stmt_type = ivl_statement_type(stmt);
  ivl_statement_t sub_stmt;

  switch (stmt_type) {

    case IVL_ST_NOOP:
      // Do nothing!
      break;

    case IVL_ST_CASE:
    case IVL_ST_CASEZ:
    case IVL_ST_CASEX:
      for (unsigned i = 0; i < ivl_stmt_case_count(stmt); i++) {
        process_statement(ivl_stmt_case_stmt(stmt, i), ffs, connections, is_ff);
      }
      break;

    // Conditional statements
    case IVL_ST_CONDIT:
      //@TODO: ivl_expr_t ivl_stmt_cond_expr(ivl_statement_t net);
      //@TODO: Can there be assigns in cond_exprs? I don't think so
      ivl_statement_t true_stmt, false_stmt;
      if ((false_stmt = ivl_stmt_cond_false(stmt))) {
        process_statement(false_stmt, ffs, connections, is_ff);
      }
      if ((true_stmt = ivl_stmt_cond_true(stmt))) {
        process_statement(true_stmt, ffs, connections, is_ff);
      }
      break;


    // Block of statements
    case IVL_ST_BLOCK:
      for (unsigned i = 0; i < ivl_stmt_block_count(stmt); i++) {
        process_statement(ivl_stmt_block_stmt(stmt, i), ffs, connections, is_ff);
      }
      break;


    case IVL_ST_WAIT:
      for (unsigned i = 0; i < ivl_stmt_nevent(stmt); i++) {
        ivl_event_t event = ivl_stmt_events(stmt, i);
        if (ivl_event_npos(event) > 0 && ivl_event_nneg(event) == 0 && ivl_event_nany(event) == 0) {
          assert(!is_ff && "Not dealing with always blocks in always block");
          is_ff = true;
        } //@TODO: Better rules about what makes a FF
      }
      if ((sub_stmt = ivl_stmt_sub_stmt(stmt))) {
        process_statement(sub_stmt, ffs, connections, is_ff);
      }
      break;

    case IVL_ST_DELAY:
    case IVL_ST_DELAYX:
      if ( (sub_stmt = ivl_stmt_sub_stmt(stmt)) ) {
        process_statement(sub_stmt, ffs, connections, is_ff);
      }
      break;

    case IVL_ST_ASSIGN_NB:
    case IVL_ST_ASSIGN:
      // If we are in a ff, no blocking assignments
      // @TODO: Check for wire and reg ff's only
      if (is_ff && (stmt_type == IVL_ST_ASSIGN)) {
        fprintf(stderr, "WARNING: Blocking assingment in flip-flop. Skipping...\n");
        fprintf(stderr, "File: %s Line: %d\n", ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
        break;
      }
      process_lvals(stmt, ffs, connections, is_ff);
      break;

    default:
      fprintf(stderr, "ERROR: Statement Type %d not yet supported\n", stmt_type);
      fprintf(stderr, "File: %s Line: %d\n", ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
      break;
  }
  return 0;
}

int process_statement(ivl_statement_t stmt, sig_map_t& ffs, std::vector<connection_t>& connections) {
  return process_statement(stmt, ffs, connections, false /*start out of ff*/);
}
