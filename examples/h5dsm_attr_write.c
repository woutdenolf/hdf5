#include "h5dsm_example.h"
#include <time.h>

int main(int argc, char *argv[]) {
    uuid_t pool_uuid;
    char *pool_grp = NULL;
    hid_t file = -1, obj = -1, attr = -1, fapl = -1;
    H5VL_daosm_snap_id_t snap_id;
    int buf[4][6];
    int i, j;

    (void)MPI_Init(&argc, &argv);
    (void)daos_init();

    /* Seed random number generator */
    srand(time(NULL));

    if(argc < 6 || argc > 7)
        PRINTF_ERROR("argc must be 6 or 7\n");

    /* Parse UUID */
    if(0 != uuid_parse(argv[1], pool_uuid))
        ERROR;

    /* Set up FAPL */
    if((fapl = H5Pcreate(H5P_FILE_ACCESS)) < 0)
        ERROR;
    if(H5Pset_fapl_daosm(fapl, MPI_COMM_WORLD, MPI_INFO_NULL, pool_uuid, pool_grp) < 0)
        ERROR;

    /* Open file */
    if((file = H5Fopen(argv[2], H5F_ACC_RDWR, fapl)) < 0)
        ERROR;

    /* Open object */
    if(!strcmp(argv[3], "-d") || !strcmp(argv[3], "-D")) {
        if((obj = H5Dopen2(file, argv[4], H5P_DEFAULT)) < 0)
            ERROR;
    }
    else {
        if(strcmp(argv[3], "-g") && strcmp(argv[3], "-G"))
            PRINTF_ERROR("argv[3] must be -d, -D, -g, or -G\n");
        if((obj = H5Gopen2(file, argv[4], H5P_DEFAULT)) < 0)
            ERROR;
    }

    /* Open attribute */
    if((attr = H5Aopen(obj, argv[5], H5P_DEFAULT)) < 0)
        ERROR;

    /* Fill and print buffer */
    printf("Writing data. Buffer is:\n");
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 6; j++) {
            buf[i][j] = rand() % 10;
            printf("%d ", buf[i][j]);
        }
        printf("\n");
    }

    /* Write data */
    if(H5Awrite(attr, H5T_NATIVE_INT, buf) < 0)
        ERROR;

    /* Save snapshot if requested */
    if(argc == 7) {
        if(H5VLdaosm_snap_create(file, &snap_id) < 0)
            ERROR;
        printf("Saved snapshot: snap_id = %llu\n", (long long unsigned)snap_id);
    } /* end if */

    /* Close */
    if(H5Aclose(attr) < 0)
        ERROR;
    if(H5Oclose(obj) < 0)
        ERROR;
    if(H5Fclose(file) < 0)
        ERROR;
    if(H5Pclose(fapl) < 0)
        ERROR;

    printf("Success\n");

    (void)daos_fini();
    (void)MPI_Finalize();
    return 0;

error:
    H5E_BEGIN_TRY {
        H5Aclose(attr);
        H5Oclose(obj);
        H5Fclose(file);
        H5Pclose(fapl);
    } H5E_END_TRY;

    (void)daos_fini();
    (void)MPI_Finalize();
    return 1;
}

