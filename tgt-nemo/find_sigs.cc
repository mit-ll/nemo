#include "find_sigs.h"

Sig_Finder::Sig_Finder(): root_scopes(NULL), num_root_scopes(0){}

Sig_Finder::Sig_Finder(ivl_scope_t* rs, unsigned int num_rs)
	: root_scopes(rs), num_root_scopes(num_rs){}

void Sig_Finder::enumerate_design_sigs(std::vector<TTB_Signal>& sigs){
	// Find all signals in a design
	for (unsigned int i = 0; i < num_root_scopes; i++) {
		enumerate_sigs(root_scopes[i], sigs);
	}
}

void Sig_Finder::find_critical_sigs(std::vector<TTB_Signal>& sigs){
	return;
}

// Enumerate all the signals in scope
void Sig_Finder::enumerate_sigs(ivl_scope_t scope, std::vector<TTB_Signal>& sigs) {
	//@TODO: Look more into dealing with scopes that are tasks or functions
	if ( ivl_scope_type(scope) != IVL_SCT_MODULE && ivl_scope_type(scope) != IVL_SCT_BEGIN && ivl_scope_type(scope) != IVL_SCT_TASK ) {
		fprintf(stderr, "ERROR: Cannot parse scope type (%d)\n", ivl_scope_type(scope));
		fprintf(stderr, "File: %s Line: %d\n", ivl_scope_file(scope), ivl_scope_lineno(scope));
		return;
	}

	// Rescurse into any submodules
	for (unsigned int i = 0; i < ivl_scope_childs(scope); i++) {
		enumerate_sigs(ivl_scope_child(scope, i), sigs);
	}

	unsigned num_sigs = ivl_scope_sigs(scope);
	TTB_Signal sig;
	for (unsigned idx = 0; idx < num_sigs; idx++) {
		sig = ivl_scope_sig(scope, idx);
		sigs.push_back(TTB_Signal(sig));

		if std::regex_search(sigs.back()->name(), critical_sigs_regex){
			printf("Found Critical Sig: %s\n", sigs.back()->name());
		}
	}
}