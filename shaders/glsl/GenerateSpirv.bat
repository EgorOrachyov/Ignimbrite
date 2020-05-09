
if not exist %cd%\CompiledSpirv mkdir CompiledSpirv

for %%f in (*.vert) do (
	glslangValidator.exe -V %%f -o CompiledSpirv/%%~nf.vert.spv
)

for %%f in (*.frag) do (
	glslangValidator.exe -V %%f -o CompiledSpirv/%%~nf.frag.spv
)

for %%f in (*.comp) do (
	glslangValidator.exe -V %%f -o CompiledSpirv/%%~nf.comp.spv
)