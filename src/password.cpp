// password.cpp
// Copyright (c) 2007 Krodo
// Part of Bylins http://www.mud.ru

#include "password.hpp"
#include "utils.h"
#include "interpreter.h"

// ��� ������� ���������� ���������� (�� ������� ����� ��������� ����� make test � �� ��������)
//#define NOCRYPT
// � ������ ������ ��� ����������� ������ ����� ������ � �������� ����
#if defined(NOCRYPT)
	#define CRYPT(a,b) (a)
#else
	#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

namespace Password {

const char *BAD_PASSWORD = "������ ������ ���� �� 10 �� 50 �������� � �� ������ ���� ������ ���������.";
const unsigned int MIN_PWD_LENGTH = 10;
const unsigned int MAX_PWD_LENGTH = 50;

/**
* ��������� ���� � �����-����� ��������� �������
*/
std::string generate_md5_hash(const std::string &pwd)
{
	char key[14];
	key[0] = '$';
	key[1] = '1';
	key[2] = '$';
	for (int i = 3; i < 12; i ++) {
		int c = number(0, 63);
		if (c < 26)
			key[i] = c + 'a';
		else if (c < 52)
			key[i] = c - 26 + 'A';
		else if (c < 62)
			key[i] = c - 52 + '0';
		else
			key[i] = '/';
	}
	key[12] = '$';
	key[13] = '\0';
	return CRYPT(pwd.c_str(), key);
}

/**
* ���������� ����� ��� � ����� ��� ����
*/
void set_password(CHAR_DATA *ch, const std::string &pwd)
{
	if (GET_PASSWD(ch))
		free(GET_PASSWD(ch));
	GET_PASSWD(ch) = str_dup(generate_md5_hash(pwd).c_str());
}

/**
* ��� ���� � ������
* \return  0 - des, 1 - md5
*/
bool get_password_type(const CHAR_DATA *ch)
{
	return CompareParam("$1$", GET_PASSWD(ch));
}

/**
* ��������� ����� � ������� ��� ������������� � ��5
* \return 0 - �� �������, 1 - �������
*/
bool compare_password(CHAR_DATA *ch, const std::string &pwd)
{
	bool result = 0;
	if (get_password_type(ch))
		result = CompareParam(ch->player.passwd, CRYPT(pwd.c_str(), GET_PASSWD(ch)), 1);
	else {
		// ���� ������ des ������� - ��������� ����� � md5 (10 - ������ MAX_PWD_LENGTH)
		if (!strncmp(CRYPT(pwd.c_str(), GET_PASSWD(ch)), GET_PASSWD(ch), 10)) {
			set_password(ch, pwd);
			result = 1;
		}
	}
	return result;
}

/**
* �������� ������ �� ����� � �������
* \return 0 - ������������ ������, 1 - ����������
*/
bool check_password(const CHAR_DATA *ch, const char *pwd)
{
	if (!pwd || !str_cmp(pwd, GET_PC_NAME(ch)) || strlen(pwd) > MAX_PWD_LENGTH || strlen(pwd) < MIN_PWD_LENGTH )
		return 0;
	return 1;
}

} // namespace Password
