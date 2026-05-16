#include "font_manager.h"

FontManager::~FontManager()
{
	clear();
}

TTF_Font* FontManager::find_font(const std::string_view& key) const
{
	FontPool::const_iterator iterator = _font_pool.find(std::string(key));
	if (iterator == _font_pool.end())
		return nullptr;

	return iterator->second;
}

void FontManager::clear()
{
	for (FontPool::value_type& font : _font_pool)
	{
		if (font.second)
			TTF_CloseFont(font.second);
	}

	_font_pool.clear();
}

size_t FontManager::resource_count() const
{
	return _font_pool.size();
}
