" Primitive syntax file for Tzara patches

if exists("b:current_syntax")
  finish
endif

syn match tzKeyword '[=+<>]'
syn region tzKeyword start='@\w*' end='\w*[ \n]'
"syn match tzConstant '='
syn region tzConstant start='= \d\+' end=' '
syn region tzConstant start='= \d\+\.\d*' end=' '
syn region tzConstant start='= [-+]\d\+' end=' '
syn region tzConstant start='= [-+]\d\+\.\d*' end=' '
syn region tzConstant start='$\w*' end='\w*[ \n]'
syn keyword tzNode module var add sub mult div clip round mix map miditofreq mem phasor pulse sinosc seq8 random segment select 
syn keyword tzIONode _out_ _in_

syn match tzComment "#.*$"
syn match tzIO "^@.*$"

hi def link tzComment Comment
hi def link tzKeyword Statement 
hi def link tzConstant Constant
hi def link tzNode Type 
hi def link tzIO Constant
hi def link tzIONode Constant

