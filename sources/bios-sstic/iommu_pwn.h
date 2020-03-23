int exp_raw_dl(char *name, unsigned char *mac, unsigned char *lip, unsigned
    char *rip);
int iommu_pwn(unsigned int addr);
int iommu_pwn_page(unsigned int low, unsigned int high);
void iommu_pwn_txt(unsigned int n);
void iommu_pwn_rootkit_linux(void);
void iommu_pwn_wait_for_bar(void);
