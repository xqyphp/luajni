#pragma once
#include <string>
#include <vector>

typedef enum {
	TypeInvalid = -1,
	TypeVoid = 0,
	TypeBoolean,
	TypeByte,
	TypeChar,
	TypeShort,
	TypeInteger,
	TypeLong,
	TypeFloat,
	TypeDouble,
	TypeString,
	TypeObject
} JavaValueType;



class JavaSignature
{
public:
	JavaSignature();
	~JavaSignature();
	bool ParseSignature(const char* sig);
	bool IsValid() const { return mIsValid; }
	bool IsField() const { return mIsField; }
	JavaValueType GetReturnType() const { return mReturnType; }
	JavaValueType GetLuaReturnType() const { return ToLuaType(mReturnType); }
	std::string GetReturnTypeName()const;
	std::vector<JavaValueType> GetArgTypes()const { return mArgTypes; }
	size_t GetArgCnt() const { return mArgTypes.size(); };
	void Reset() { mIsValid = false; };
	
protected:
	static JavaValueType CheckType(const std::string &sig, size_t *pos);
	static JavaValueType ToLuaType(JavaValueType type);
private:
	bool mIsValid;
	bool mIsField;

	std::string mSignatureStr;
	std::string mReturnSignature;

	JavaValueType mReturnType;
	std::vector<JavaValueType> mArgTypes;
};

