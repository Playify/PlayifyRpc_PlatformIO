

template<typename T>
struct MultipleArguments: std::vector<T>{
	MultipleArguments() = default;

	MultipleArguments(std::initializer_list<T> args):std::vector<T>(args){}

	explicit MultipleArguments(std::vector<T> args):std::vector<T>(args){}
};