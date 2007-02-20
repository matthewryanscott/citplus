:cl /c /Gf /Ge /Zi /MTd /FR /DVISUALC /DWINCIT %1
:for %%f in (%1) do insure /c /Gf /Ge /Zi /MT /DVISUALC /DWINCIT %%f
nmcl /c /Gf /Ge /Zi /MTd /DVISUALC /DWINCIT %1
