#
# Copyright 2017-2018, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#
#     * Neither the name of the copyright holder nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

param (
  [string]$URL,
  [string]$PATH,
  [string]$SHA256HASH
)

# disable Error printing when command fails
$ErrorActionPreference = "SilentlyContinue"

# Replace "/" with "\" so Split-Path can work with replace
$PATH = $PATH.replace("/", "\")

# Get main repo directory and short destination path
$REPODIR = (Split-Path $PSScriptRoot | Split-Path)
$SHORTPATH = $PATH.replace("$REPODIR", "")

# Checks if file is correct (exists and has correct SHA256 checksum)
Function Is-File-Correct
{
	if (-Not (Test-Path "$PATH"))
	{
		return $false
	}

	$HASH = (Get-FileHash "$PATH" -Algorithm SHA256 | Select-Object -ExpandProperty Hash)
	if ("$HASH" -eq "$SHA256HASH")
	{
		return $true
	}
	return $false
}

# If file is already downloaded then exit
if (Is-File-Correct)
{
	exit 0
}

# Create download destination directory if it does not exist
$DIRTOCREATE = (Split-Path $PATH)

if (-Not (Test-Path "$DIRTOCREATE"))
{
	New-Item $DIRTOCREATE -ItemType directory -Force | Out-Null
}

Write-Host -NoNewLine "Downloading file $URL to $SHORTPATH... "

# If file is incorrect then download it again
for ($i = 1; $i -le 10; $i++)
{
	(New-Object Net.WebClient).DownloadFile("${URL}", "${PATH}")
	if (Is-File-Correct)
	{
		Write-Host "success"
		exit 0
	}
}

exit 1
