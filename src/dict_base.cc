#ifndef DICTBASE_H
#define DICTBASE_H

#include <napi.h>

using namespace Napi;

class DictBase {
  private:
	mutable Object _obj;
	Env _env;

  public:
	DictBase(Env env, Object source)
	  : _obj(source)
	  , _env(env) {
	}

	DictBase(Env env)
	  : _env(env) {
		_obj = Object::New(env);
	}

	~DictBase() {
	}

	Value GetMember(const char* name) const {
		return _obj.Get(name);
	}

	Value GetMember(const std::string& name) const {
		return GetMember(name.c_str());
	}

	void SetMemberUndefined(const char* name) {
		_obj.Set(name, _env.Undefined());
	}

	void DeleteMember(const char* name) {
		_obj.Delete(name);
	}

	void SetMember(const char* name, const char* value) {
		_obj.Set(name, String::New(_env, value));
	}

	void SetMember(const char* name, Value value) {
		_obj.Set(name, value.As<String>());
	}

	void SetMember(const char* name, const std::string& value) {
		_obj.Set(name, String::New(_env, value.c_str()));
	}

	template<typename T, typename V, uint32_t len>
	void SetMemberArray(const char* name, V value[len]) {
		Object array = Array::New(_env, len);
		for (uint32_t i = 0; i < len; i++) {
			array.Set(i, Value::From(this->_env, value[i]));
		}
		SetMember(name, array);
	}

	template<typename T>
	void SetMemberT(const char* name, const T& value) {
		_obj.Set(name, Value::From(this->_env, value));
	}

	bool IsMemberPresent(const char* name) const {
		return _obj.Has(name);
	}

	bool IsMemberPresent(const std::string& name) const {
		return IsMemberPresent(name.c_str());
	}

	Object GetObject() const {
		return _obj;
	}
};

#endif
