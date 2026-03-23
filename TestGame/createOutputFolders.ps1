$currentfolder=&"pwd"
$debug_folders = @("Debug", "Editor_Debug")
$release_folders = @("Release", "Development", "Editor_Release", "Editor_Development", "Editor_SoftDebug")
#$debug_folders = @("Editor_Debug")
#$release_folders = @()
$folders = $debug_folders + $release_folders
#$folders = @("Editor_SoftDebug")
$srcfolder = "Target"
$ignorelist= @(".gitignore")

function SetupFolder {
	param (
		[string]$folder
	)
	
	cmd /c mkdir $folder
	
	$items = Get-ChildItem -Path $srcfolder

	foreach ($item in $items) {
		#ignore things from the ignorelist
		if($item -in $ignorelist) { 
			continue
		}
		
		$symlink = "$($currentfolder)\$($folder)\$($item.Name)"
		$srcfile = "$($currentfolder)\$($srcfolder)\$($item.Name)"

		# Example: Check if it's a file or a directory
		if ($item.PSIsContainer) {
			Write-Host "Directory Symlink $($srcfile) -> $($symlink)"
			cmd /c mklink /D $symlink $srcfile
		} else {
			Write-Host "File Symlink $($srcfile) -> $($symlink)"
			cmd /c mklink $symlink $srcfile
		}
	}
}

foreach ($folder in $folders) {
	SetupFolder $folder
}