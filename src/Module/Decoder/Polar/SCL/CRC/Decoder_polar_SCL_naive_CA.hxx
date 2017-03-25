#include <stdexcept>

#include "Decoder_polar_SCL_naive_CA.hpp"

namespace aff3ct
{
namespace module
{
template <typename B, typename R, proto_f<R> F, proto_g<B,R> G>
Decoder_polar_SCL_naive_CA<B,R,F,G>
::Decoder_polar_SCL_naive_CA(const int& K, const int& N, const int& L, const mipp::vector<B>& frozen_bits, CRC<B>& crc,
                             const int n_frames, const std::string name)
: Decoder_polar_SCL_naive<B,R,F,G>(K, N, L, frozen_bits, n_frames, name), crc(crc)
{
	assert(crc.size());

	if (!crc.size())
		throw std::invalid_argument("aff3ct::module::Decoder_polar_SCL_naive_CA: \"crc.size()\" has to be greater "
		                            "than 0.");
	if (crc.size() > K)
		throw std::invalid_argument("aff3ct::module::Decoder_polar_SCL_naive_CA: \"crc.size()\" has to be equal or "
		                            "smaller than K.");
}

template <typename B, typename R, proto_f<R> F, proto_g<B,R> G>
void Decoder_polar_SCL_naive_CA<B,R,F,G>
::select_best_path() 
{
	mipp::vector<B> U_test;
	std::set<int> active_paths_before_crc = this->active_paths;
	for (auto path : active_paths_before_crc)
	{
		U_test.clear();

		for (auto leaf = 0 ; leaf < this->N ; leaf++)
			if (!this->frozen_bits[leaf]) 
				U_test.push_back(this->leaves_array[path][leaf]->get_c()->s[0]);

		bool decode_result = crc.check(U_test, this->get_simd_inter_frame_level());
		if (!decode_result)
			this->active_paths.erase(path);
	}

	this->Decoder_polar_SCL_naive<B,R,F,G>::select_best_path();
}
}
}
