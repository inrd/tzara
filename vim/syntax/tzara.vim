" Primitive syntax file for Tzara patches

if exists("b:current_syntax")
  finish
endif

syn match tzKeyword '[+>]'
syn region tzKeyword start='@\w*' end='\w*[ \n]'
"syn match tzConstant '='
syn region tzConstant start='= \d\+' end=' '
syn region tzConstant start='= \d\+\.\d*' end=' '
syn region tzConstant start='= [-+]\d\+' end=' '
syn region tzConstant start='= [-+]\d\+\.\d*' end=' '
syn region tzConstant start='$\w*' end='\w*[ \n]'
syn keyword tzNode out var add sub mult div clip round mix map miditofreq mem phasor pulse sinosc seq8 random segment select

syn match tzComment "#.*$"

hi def link tzComment Comment
hi def link tzKeyword Statement 
hi def link tzConstant Constant
hi def link tzNode Type 

