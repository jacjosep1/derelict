// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <memory>
#include <string>

#include "Algorithms/array2D.h"

namespace DebugPrinting {
	static constexpr bool DISABLE_DEBUG{ true };

	inline static void PrintBool(bool b, const std::string &msg="") {
		if (DISABLE_DEBUG) return;
		FString s = TEXT("Debug bool: ");
		s.Append(UTF8_TO_TCHAR(msg.c_str()));
		s.AppendInt(static_cast<int>(b));
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Red, s);
	}

	inline static void PrintSizet(size_t b, const std::string& msg = "") {
		if (DISABLE_DEBUG) return;
		FString s = TEXT("Debug bool: ");
		s.Append(UTF8_TO_TCHAR(msg.c_str()));
		s.Append(LexToString(b));
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Red, s);
	}

	inline static void PrintInt(int b, const std::string& msg = "") {
		if (DISABLE_DEBUG) return;
		FString s = TEXT("Debug int: ");
		s.Append(UTF8_TO_TCHAR(msg.c_str()));
		s.Append(LexToString(b));
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Blue, s);
	}

	inline static void PrintChar(char b, const std::string& msg = "") {
		if (DISABLE_DEBUG) return;
		FString s = TEXT("Debug char: ");
		s.Append(UTF8_TO_TCHAR(msg.c_str()));
		s.AppendChar(b);
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Cyan, s);
	}
	
	inline static void PrintLocation(location_t b, const std::string& msg = "") {
		if (DISABLE_DEBUG) return;
		FString s = TEXT("Debug location: ");
		s.Append(UTF8_TO_TCHAR(msg.c_str()));
		s.Append(LexToString(b.x));
		s.Append(",");
		s.Append(LexToString(b.y));
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Blue, s);
	}

	template<typename T> inline static void PrintShared(std::shared_ptr<T> b, const std::string& msg="") {
		if (DISABLE_DEBUG) return;
		FString s = TEXT("Debug shared: ");
		s.Append(UTF8_TO_TCHAR(msg.c_str()));
		s.Append(LexToString(reinterpret_cast<size_t>(b.get()) & 0xFFFF));
		GEngine->AddOnScreenDebugMessage(-1, 999.f, FColor::Emerald, s);
	}
}
