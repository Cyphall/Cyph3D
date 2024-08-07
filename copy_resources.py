import pathlib
import os
import sys
import shutil
import posixpath
import filecmp
import subprocess


def copy_file(input_path, output_path):
	if not os.path.exists(output_path) or os.stat(input_path).st_mtime > os.stat(output_path).st_mtime:
		shutil.copyfile(input_path, output_path)


def compile_shader(input_path, output_path):
	print(f"Building shader file {input_path}")
	
	output_path = f"{output_path}.spv"
	
	arguments = [compiler_path, input_path]
	arguments.extend(["-o", output_path])
	arguments.extend(["--target-env", "vulkan1.3"])
	arguments.extend(["--quiet"])
	if cmakeConfig == "Debug" or cmakeConfig == "RelWithDebInfo":
		arguments.extend(["-gVS"])

	result = subprocess.run(arguments)
	if result.returncode != 0:
		sys.exit(1)


def process_file(input_path: str):
	ext = posixpath.splitext(input_path)[1]
	relative_path = posixpath.relpath(input_path, source_dir)
	output_path = posixpath.join(build_dir, relative_path)

	if posixpath.exists(output_path) and filecmp.cmp(input_path, output_path, shallow=True):
		return

	os.makedirs(posixpath.dirname(output_path), exist_ok=True)

	if ext == ".vert":
		compile_shader(input_path, output_path)
	elif ext == ".frag":
		compile_shader(input_path, output_path)
	elif ext == ".geom":
		compile_shader(input_path, output_path)
	elif ext == ".comp":
		compile_shader(input_path, output_path)
	elif ext == ".rgen":
		compile_shader(input_path, output_path)
	elif ext == ".rmiss":
		compile_shader(input_path, output_path)
	elif ext == ".rchit":
		compile_shader(input_path, output_path)
	elif ext == ".rahit":
		compile_shader(input_path, output_path)
	elif ext == ".rint":
		compile_shader(input_path, output_path)
	elif ext == ".glsl":
		pass # glsl common files, not compiled as is but included in other shader files
	else:
		copy_file(input_path, output_path)


if len(sys.argv) != 5:
	sys.exit(1)

source_dir = sys.argv[1]
build_dir = sys.argv[2]
compiler_path = sys.argv[3]
cmakeConfig = sys.argv[4]

for path, subdirs, files in os.walk(source_dir):
	for name in files:
		process_file(pathlib.Path(path, name).as_posix())