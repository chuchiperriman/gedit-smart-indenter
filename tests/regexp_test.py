# -*- mode: python; tab-width: 4; indent-tabs-mode: nil -*- */

#!/usr/bin/env python

import re
si = [
	"{",
	"	{",
	"{	",
	"if (a == B) {",
	"if (a==b){ asdfasdf;",
	"if (a==b){aaa;}{",
]

no = [
	"if (a==b){}  ",
	"if (a==b){print ();}  ",
	"lalala;",
	"}",
	"	}	",
	""
	"print ();}"
]

reg = r'.*\{[^\}]*$';

print '#' * 50

for t in si:
	m = re.match (reg, t);
	if m:
		print '-' * 50
	else:
		print '******error*******'

	print '[' + t +']'

print '#' * 50

for t in no:
	m = re.match (reg, t);
	if m == None:
		print '-' * 50
	else:
		print '******error*******'
		print 'match:' , m.group ()

	print '[' + t + ']'

params = [
	"	tab_changed_cb (GeditWindow *geditwindow,",
]

regs = [r'.*\([^\)]*$', r'.*,\s*$']

for t in params:
	found = False
	for r in regs:
		m = re.match (r, t);
		if m:
			found = True
	if found:
		print '-' * 50
	else:
		print '******error*******'
	print '[' + t + ']'





