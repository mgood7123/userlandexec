set -v
rm -rfv *.o *.so *.a e*
rm ./example* -rfv *.so
gcc -fPIC -pie -shared -I. -g3 -std=gnu99  ul_exec_libc.c -o ul_exec_libc.o -c
gcc -fPIC -pie -shared -I. -g3 -std=gnu99  ul_exec_libc.c -o ul_exec_libc.so
ar rcs ul_exec_libc.a ul_exec_libc.o
gcc -g3 ul_example.c ./ul_exec_libc.so -o example
gcc -fPIC --pie -g3 ul_example.c ul_exec_libc.a -o example_

echo
echo
./example
#echo
#echo
#./example_ /bin/ls   -l
#echo
#echo
# ldd ./example
# file ./example
# ldd ./elfauxv_dynamic
# file ./elfauxv_dynamic
# ldd ./elfauxv_static
# file ./elfauxv_static
# ldd /bin/ls
# file /bin/ls

#non important stuff
# gcc -I. -g -std=gnu99 -nostdlib static_dyn_load_run_.c -o static_dyn_load_run -Llibstatic -lstatic
# gcc -I. -g -std=gnu99 -nostdlib dyn_unmap_run_.c -o dyn_unmap_run -Llibstatic -lstatic
# gcc -I. -g -std=gnu99 -nostdlib hw.c -o hw -Llibstatic -lstatic
# chmod +x hw
# gcc -g -I. -std=gnu99 -nostdlib -fPIC env_test.c -o env_test -Llibstatic -lstatic
# chmod +x env_test
# gcc -g -I. -std=gnu99 -nostdlib -fPIC margs.c -o margs -Llibstatic -lstatic
# chmod +x margs
# gcc -I. -g -Wall -Wextra -o args2 args2.c
# chmod +x args2
# gcc -I. -g -std=gnu99 -nostdlib elfauxv.c -o elfauxv -Llibstatic -lstatic
# chmod +x elfauxv
# gcc -I. -g -Wall -Wextra -o elfauxv_dynamic elfauxv_dynamic.c
# chmod +x elfauxv_dynamic
# gcc -I. -g -Wall -Wextra -o example example.c -ldl
# gcc -fPIC -shared -I. -g -std=gnu99 -nostdlib ulexec_.c -o ulexec.so -Llibstatic -lstatic
