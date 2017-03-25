#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "Tools/Display/bash_tools.h"

#include "Decoder_RA.hpp"

using namespace aff3ct::module;

template <typename B, typename R>
Decoder_RA<B, R>
::Decoder_RA(const int& K, const int& N, Interleaver<int>& interleaver, int max_iter, const int n_frames,
             const std::string name)
: Decoder<B,R>(K, N, n_frames, 1, name),
  rep_count(N/K),
  max_iter(max_iter),
  Y_N(N),
  Fw(N),
  Bw(N),
  V_K(K),
  Tu(N),
  Td(N),
  Wu(N),
  Wd(N),
  U(K),
  Xd(2),
  Xu(2),
  interleaver(interleaver)
{
	if (max_iter <= 0)
		throw std::invalid_argument("aff3ct::module::Decoder_RA: \"max_iter\" has to be greater than 0.");
	if (N % K)
		throw std::invalid_argument("aff3ct::module::Decoder_RA: \"K\" has to be a multiple of \"N\".");
	if ((int)interleaver.size() != N)
		throw std::length_error("aff3ct::module::Decoder_RA: \"interleaver.size()\" has to be equal to \"N\".");

	Xd[0].resize(N);
	Xd[1].resize(N);
	Xu[0].resize(N);
	Xu[1].resize(N);
}

template <typename B, typename R>
Decoder_RA<B, R>
::~Decoder_RA()
{
}

template <typename B, typename R>
void Decoder_RA<B, R>
::_load(const mipp::vector<R>& Y_N)
{
	std::copy(Y_N.begin(), Y_N.begin() + this->Y_N.size(), this->Y_N.begin());
}

template <typename B, typename R>
void Decoder_RA<B, R>
::_hard_decode()
{
	//set F, B and Td at 0
	for (auto i = 0; i < this->N; i++)
	{
		Fw[i] = 0;
		Bw[i] = 0;
		Td[i] = 0;
	}

	for (auto iter = 0; iter < max_iter; iter++)
	{
		///////////////////
		// SISO decoding //
		///////////////////

		// Forward
		Fw[0] = Td[0];
		for (auto i = 1; i < this->N; i++)
			Fw[i] = check_node(Fw[i - 1] + Y_N[i - 1], Td[i]);

		// Backward
		Bw[this->N - 2] = check_node(Y_N[this->N - 1], Td[this->N - 1]);

		for (auto i = this->N - 3; i >= 0; i--)
			Bw[i] = check_node(Bw[i + 1] + Y_N[i + 1], Td[i + 1]);

		// Extrinsic
		Tu[0] = Bw[0] + Y_N[0];
		Tu[this->N - 1] = check_node(Y_N[this->N - 1], Y_N[this->N - 2] + Fw[this->N - 2]);
		for (auto i = 1; i < this->N - 1; i++)
			Tu[i] = check_node(Fw[i - 1] + Y_N[i - 1], Bw[i] + Y_N[i]);

		// Deinterleave
		interleaver.deinterleave(Tu, Wu, false, 1);

		// U computation
		R tmp;
		for (auto i = 0; i < this->K; i++)
		{
			tmp = 0;
			for (auto j = 0; j < rep_count; j++)
				tmp += Wu[i * rep_count + j];
			for (auto j = 0; j < rep_count; j++)
				Wd[i * rep_count + j] = tmp - Wu[i * rep_count + j];
			U[i] = tmp;
		}

		// Interleaving
		interleaver.interleave(Wd, Td, false, 1);
	}
	for (auto i = 0; i < this->K; i++)
		V_K[i] = (U[i] > 0) ? 0 : 1;
}

template <typename B, typename R>
void Decoder_RA<B, R>
::_store(mipp::vector<B>& V_K) const
{
	V_K = this->V_K;
}

template <typename B, typename R>
R Decoder_RA<B, R>
::check_node(R a, R b)
{
	B sign1, sign2, resSign;
	R resMag, res;
	R mag1, mag2;

	sign1 = sign(a);
	sign2 = sign(b);
	mag1 = std::abs(a);
	mag2 = std::abs(b);
	resMag = std::min(mag1, mag2);
	resSign = (sign1 != sign2) ? -1 : 1;

	res = (resSign == 1) ? resMag : -resMag;

	return res;
}

template <typename B, typename R>
int Decoder_RA<B, R>
::sign(R x)
{
	return (x > 0) ? 1 : -1;
}

// ==================================================================================== explicit template instantiation
#include "Tools/types.h"
#ifdef MULTI_PREC
template class aff3ct::module::Decoder_RA<B_8,Q_8>;
template class aff3ct::module::Decoder_RA<B_16,Q_16>;
template class aff3ct::module::Decoder_RA<B_32,Q_32>;
template class aff3ct::module::Decoder_RA<B_64,Q_64>;
#else
template class aff3ct::module::Decoder_RA<B,Q>;
#endif
// ==================================================================================== explicit template instantiation
