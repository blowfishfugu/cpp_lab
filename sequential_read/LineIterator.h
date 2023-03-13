#pragma once
#include <string_view>
#include <atomic>
#include <mutex>
#include <optional>

/* IteratorTraits, nachschlagbar in <xutility>
template<class _Iter>
	struct _Iterator_traits_base<_Iter, void_t<
		typename _Iter::iterator_category,
		typename _Iter::value_type,
		typename _Iter::difference_type,
		typename _Iter::pointer,
		typename _Iter::reference
		>>
	{	// defined if _Iter::* types exist
	using iterator_category = typename _Iter::iterator_category;
	using value_type = typename _Iter::value_type;
	using difference_type = typename _Iter::difference_type;

	using pointer = typename _Iter::pointer;
	using reference = typename _Iter::reference;
	};
*/

struct my_line_iterator
{
	using iterator_category = std::forward_iterator_tag;
	using value_type = std::string_view;
	using difference_type = std::ptrdiff_t;
	using reference_type = value_type const&;
	using pointer_type = const value_type*;

	std::optional<std::string_view> theText{};//container
	value_type theLine;//value (*it)
	size_t start_pos{};
	size_t end_pos{};
	//ctor "end"
	my_line_iterator()
	{}
	//ctor "begin"
	my_line_iterator(reference_type txt)
		: theText(txt)
	{
		++*this;
	}

	//retrieve
	reference_type operator*() const { return theLine; }
	pointer_type operator->() const { return &theLine; }

	//assign
	my_line_iterator& operator=(reference_type vw)
	{
		theText = vw;
		start_pos = 0LL;
		end_pos = 0LL;
		return *this;
	}
	my_line_iterator& operator=(my_line_iterator const& ref)
	{
		theText = ref.theText;
		theLine = ref.theLine;
		start_pos = ref.start_pos;
		end_pos = ref.end_pos;
		return *this;
	}

	//input_iterator_tag,
	//++it
	my_line_iterator& operator++() {
		theLine = {};
		if (theText) {
			end_pos = theText->find('\n', start_pos);
			if(end_pos!=std::string_view::npos){
				theLine = theText->substr(start_pos, end_pos - start_pos);
				start_pos = end_pos + 1;
			}
			else {
				theText = {};
			}
		}
		return *this;
	}
	//it++
	my_line_iterator& operator++(int) {
		my_line_iterator elem(*this);
		++*this;
		return elem;
	}
	//compare
	friend bool operator==(my_line_iterator const& lhs, my_line_iterator const& rhs)
	{
		return lhs.theText == rhs.theText;
	}
	
	friend bool operator!=(my_line_iterator const& lhs, my_line_iterator const& rhs)
	{
		return !(lhs == rhs);
	}
};

struct my_lines {
	std::string_view theText;
	my_lines(std::string_view const& input)
	{ 
		theText = input; 
	}
	my_lines(my_lines const& ref) 
	{ 
		theText = ref.theText; 
	}
	my_line_iterator begin() const { return my_line_iterator(theText); }
	my_line_iterator end() const { return my_line_iterator{}; }
};

template <typename ty>
struct my_line {
	std::string_view theText;
	ty data;

	my_line(void) :theText(), data() {}
	my_line(std::string_view const& input) :theText(input), data() {}

	operator ty && () { return std::forward<ty>(data); }
};

struct my_line_count {
	std::string_view theText;
	int index{};

	my_line_count(void) :theText(), index(counter++) {}
	static int getCounter() { return counter; }
	static inline std::atomic<int> counter = 0;
	static void reset(void) { counter = 0; }
};
