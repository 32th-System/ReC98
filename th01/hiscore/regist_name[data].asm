public _REGIST_NAME_BLANK, _REGIST_NAME_SPACES
_REGIST_NAME_BLANK	db '�Q�Q�Q�Q�Q�Q�Q�Q',0
if BINARY eq 'M'
	public _regist_jump_to_enter
	_regist_jump_to_enter	db 0
endif
_REGIST_NAME_SPACES	db '�@�@�@�@�@�@�@�@',0
