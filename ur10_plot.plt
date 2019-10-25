set datafile missing 'NaN'

delay = 0.075

plot \
  'z.ur10.log' us (strcol(1) eq "REAL" ? ($2-delay) : 0/0):3 w lp t 'real1',\
  'z.ur10.log' us (strcol(1) eq "REAL" ? ($2-delay) : 0/0):4 w lp t 'real2',\
  'z.ur10.log' us (strcol(1) eq "REAL" ? ($2-delay) : 0/0):5 w lp t 'real3',\
  'z.ur10.log' us (strcol(1) eq "REAL" ? ($2-delay) : 0/0):6 w lp t 'real4',\
  'z.ur10.log' us (strcol(1) eq "REAL" ? ($2-delay) : 0/0):7 w lp t 'real5',\
  '' us (strcol(1) eq "REF" ? $17 : NaN):3 w lp t 'ref1',\
  '' us (strcol(1) eq "REF" ? $17 : NaN):4 w lp t 'ref2',\
  '' us (strcol(1) eq "REF" ? $17 : NaN):5 w lp t 'ref3',\
  '' us (strcol(1) eq "REF" ? $17 : NaN):6 w lp t 'ref4',\
  '' us (strcol(1) eq "REF" ? $17 : NaN):7 w lp t 'ref5',\
  