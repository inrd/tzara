" Primitive syntax file for Tzara patches

if exists("b:current_syntax")
  finish
endif

syn match tzKeyword '[+<>]'
syn region tzKeyword start='@\w*' end='\w*[ \n]'
"syn match tzConstant '='
syn region tzConstant start='= pi' end=' '
syn region tzConstant start='= twopi' end=' '
syn region tzConstant start='= chromatic' end=' '
syn region tzConstant start='= major' end=' '
syn region tzConstant start='= minor' end=' '
syn region tzConstant start='= harmonic_major' end=' '
syn region tzConstant start='= harmonic_minor' end=' '
syn region tzConstant start='= locrian' end=' '
syn region tzConstant start='= pyramid_hexatonic' end=' '
syn region tzConstant start='= kung' end=' '
syn region tzConstant start='= hira_joshi' end=' '
syn region tzConstant start='= ritsu' end=' '
syn region tzConstant start='= mela_citrambari' end=' '
syn region tzConstant start='= raga_bilwadala' end=' '
syn region tzConstant start='= maqam_hijaz' end=' '
syn region tzConstant start='= gnossiennes' end=' '
syn region tzConstant start='= [cdefgab]' end=' '
syn region tzConstant start='= \d\+' end=' '
syn region tzConstant start='= \d\+\.\d*' end=' '
syn region tzConstant start='= [-+]\d\+' end=' '
syn region tzConstant start='= [-+]\d\+\.\d*' end=' '
syn region tzConstant start='$\w*' end='\w*[ \n]'
syn keyword tzNode module defaultval var add sub mult div modulo pow sqrt abs sin cos tan tanh clip wrap equal nequal lower greater min max round ceil floor frac and or xor mix merge pmerge map smooth miditofreq dbtoamp mstohz hztoms samplerate fixdenorm fixnan count phasor pulse sinosc sawosc sqrosc triosc noise seq8 random notescale segment select route sah timepoint lowpass highpass svf delay fdelay
syn keyword tzIONode _out_ _in_

syn match tzComment "#.*$"
syn match tzMetadata "!.*$"
syn match tzIO "^@.*$"

hi def link tzComment Comment
hi def link tzKeyword Statement 
hi def link tzConstant Constant
hi def link tzNode Type 
hi def link tzIO Constant
hi def link tzIONode Constant
hi def link tzMetadata PreProc

