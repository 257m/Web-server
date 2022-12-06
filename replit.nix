{ pkgs }: {
	deps = [
		pkgs.lld_13
  pkgs.llvmPackages_13.llvm
  		pkgs.unzip
  		pkgs.clang_12
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
		pkgs.wabt
	];
}