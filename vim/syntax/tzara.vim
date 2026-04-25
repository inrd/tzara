" Primitive syntax file for Tzara patches

if exists("b:current_syntax")
  finish
endif

syn match tzKeyword '[+<>=]'
syn match tzPort '@\w\+'
syn match tzVar '\$\w\+'
syn match tzPath '<[^>]\+>'

syn match tzNote "\<[cdefgab]#\?-\?\d\+\>"
syn match tzNumber "\<\d\+\(\.\d\+\)\?\>"
syn match tzNumber "\s\zs[-+]\d\+\(\.\d\+\)\?\>"

syn keyword tzConstant pi twopi
syn keyword tzConstant chromatic major minor harmonic_major harmonic_minor
syn keyword tzConstant locrian pyramid_hexatonic kung hira_joshi ritsu
syn keyword tzConstant mela_citrambari raga_bilwadala maqam_hijaz gnossiennes

syn keyword tzNode module matrix mget mset defaultval var add sub mult div modulo pow sqrt abs sin cos tan tanh clip wrap equal nequal lower greater min max round ceil floor frac and or xor mix merge pmerge map from0_1 to0_1 smooth miditofreq dbtoamp mstohz hztoms samplerate duration fixdenorm fixnan count phasor pulse sinosc sawosc sqrosc triosc noise seq8 random irandom notescale segment adenv asrenv select route sah gate timepoint lowpass highpass lowpass2 highpass2 bandpass notch peak lowshelf highshelf svf delay fdelay allpass
syn keyword tzIONode _out_ _in_

syn match tzComment "#.*$"
syn match tzMetadata "!.*$"
syn match tzIO "^@.*$"

hi def link tzComment  Comment
hi def link tzKeyword  Statement
hi def link tzPort     Statement
hi def link tzConstant Constant
hi def link tzNote     Constant
hi def link tzNumber   Constant
hi def link tzVar      Constant
hi def link tzPath     String
hi def link tzNode     Type
hi def link tzIO       Constant
hi def link tzIONode   Constant
hi def link tzMetadata PreProc

let b:current_syntax = "tzara"
