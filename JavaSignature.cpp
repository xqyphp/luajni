#include "JavaSignature.h"

using namespace std;

JavaSignature::JavaSignature()
	: mIsValid(false), mIsField(false)
{
}

JavaSignature::~JavaSignature()
{
}

bool JavaSignature::ParseSignature(const char* descriptor)
{
	mSignatureStr = std::string(descriptor);
	size_t sigSize = mSignatureStr.size();
	if (sigSize < 1) {
		return false;
	}

	if (sigSize < 3 || descriptor[0] != '(') // min sig is "()V"
	{
		size_t pos = 0;
		JavaValueType type = CheckType(descriptor, &pos);
		if (type == TypeInvalid) {
			return false;
		}

		this->mReturnType = type;
		this->mIsField = true;
		this->mReturnSignature = std::string(descriptor, 0, pos + 1);

		this->mIsValid = true;
		return true;
	}

	size_t pos = 1;
	while (pos < sigSize && descriptor[pos] != ')') {
		JavaValueType type = CheckType(descriptor, &pos);
		if (type == TypeInvalid) {
			return false;
		}
		this->mArgTypes.push_back(type);
		pos++;
	}


	if (pos >= sigSize || descriptor[pos] != ')') {
		return false;
	}
	pos++;
	size_t old_pos = pos;
	this->mReturnType = CheckType(descriptor, &pos);

	mReturnSignature = std::string(descriptor, old_pos, pos + 1);

	this->mIsValid = true;
	return true;
}

std::string JavaSignature::GetReturnTypeName() const
{
	if (mReturnSignature.size() < 3) {
		return mReturnSignature;
	}
	return mReturnSignature.substr(1, mReturnSignature.size() - 2);
}

JavaValueType JavaSignature::CheckType(const std::string & sig, size_t * pos)
{
	//TODO
	switch (sig[*pos]) {
	case 'I':
		return TypeInteger;
	case 'F':
		return TypeFloat;
	case 'Z':
		return TypeBoolean;
	case 'V':
		return TypeVoid;
	case 'L':
		size_t pos2 = sig.find_first_of(';', *pos + 1);
		if (pos2 == string::npos) {
			return TypeInvalid;
		}

		const string t = sig.substr(*pos, pos2 - *pos + 1);
		if (t.compare("Ljava/lang/String;") == 0) {
			*pos = pos2;
			return TypeString;
		}
		else {
			*pos = pos2;
			return TypeObject;
		}
	}

	return TypeInvalid;
}

JavaValueType JavaSignature::ToLuaType(JavaValueType type)
{
	switch (type)
	{
	case TypeByte:
	case TypeShort:
	case TypeInteger:
	case TypeLong:
		return TypeInteger;
	case TypeDouble:
	case TypeFloat:
		return TypeFloat;
	default:
		return type;
	}
}
