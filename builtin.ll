; ModuleID = 'testcases/IR-1/builtin/builtin.c'
source_filename = "testcases/IR-1/builtin/builtin.c"
target datalayout = "e-m:e-p:32:32-i64:64-n32-S128"
target triple = "riscv32-unknown-unknown-elf"

@.str = private unnamed_addr constant [3 x i8] c"%s\00", align 1
@.str.1 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@.str.2 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.3 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind optnone
define dso_local void @print(ptr noundef %0) #0 {
  %2 = alloca ptr, align 4
  store ptr %0, ptr %2, align 4
  %3 = load ptr, ptr %2, align 4
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str, ptr noundef %3)
  ret void
}

declare dso_local i32 @printf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind optnone
define dso_local void @println(ptr noundef %0) #0 {
  %2 = alloca ptr, align 4
  store ptr %0, ptr %2, align 4
  %3 = load ptr, ptr %2, align 4
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, ptr noundef %3)
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @printInt(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %3)
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local void @printlnInt(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %3 = load i32, ptr %2, align 4
  %4 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, i32 noundef %3)
  ret void
}

; Function Attrs: noinline nounwind optnone
define dso_local ptr @getString() #0 {
  %1 = alloca ptr, align 4
  %2 = call ptr @malloc(i32 noundef 256) #5
  store ptr %2, ptr %1, align 4
  %3 = load ptr, ptr %1, align 4
  %4 = call i32 (ptr, ...) @scanf(ptr noundef @.str, ptr noundef %3)
  %5 = load ptr, ptr %1, align 4
  ret ptr %5
}

; Function Attrs: allocsize(0)
declare dso_local ptr @malloc(i32 noundef) #2

declare dso_local i32 @scanf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind optnone
define dso_local i32 @getInt() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (ptr, ...) @scanf(ptr noundef @.str.2, ptr noundef %1)
  %3 = load i32, ptr %1, align 4
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone
define dso_local ptr @builtin_memset(ptr noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca ptr, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 4
  store i32 %1, ptr %5, align 4
  store i32 %2, ptr %6, align 4
  %7 = load ptr, ptr %4, align 4
  %8 = load i32, ptr %5, align 4
  %9 = trunc i32 %8 to i8
  %10 = load i32, ptr %6, align 4
  call void @llvm.memset.p0.i32(ptr align 1 %7, i8 %9, i32 %10, i1 false)
  ret ptr %7
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i32(ptr nocapture writeonly, i8, i32, i1 immarg) #3

; Function Attrs: noinline nounwind optnone
define dso_local ptr @builtin_memcpy(ptr noundef %0, ptr noundef %1, i32 noundef %2) #0 {
  %4 = alloca ptr, align 4
  %5 = alloca ptr, align 4
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 4
  store ptr %1, ptr %5, align 4
  store i32 %2, ptr %6, align 4
  %7 = load ptr, ptr %4, align 4
  %8 = load ptr, ptr %5, align 4
  %9 = load i32, ptr %6, align 4
  call void @llvm.memcpy.p0.p0.i32(ptr align 1 %7, ptr align 1 %8, i32 %9, i1 false)
  ret ptr %7
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i32(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i32, i1 immarg) #4

attributes #0 = { noinline nounwind optnone "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+a,+c,+m,+relax,-save-restore" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+a,+c,+m,+relax,-save-restore" }
attributes #2 = { allocsize(0) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+a,+c,+m,+relax,-save-restore" }
attributes #3 = { argmemonly nocallback nofree nounwind willreturn writeonly }
attributes #4 = { argmemonly nocallback nofree nounwind willreturn }
attributes #5 = { allocsize(0) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"ilp32"}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{i32 1, !"SmallDataLimit", i32 8}
!4 = !{!"Ubuntu clang version 15.0.7"}
