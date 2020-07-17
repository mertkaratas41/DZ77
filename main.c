#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char string[] = "";
FILE * file;

/*
    LZ77 Sýkýþtýrma Algoritmasý
*/


struct lz77_dizi {
    char harf;
    int index;
    int uzunluk;
};
struct lz77_dizi *ilk;
struct lz77_dizi *son;


void lz77_yazdir(int i, int n, int k, int dizi_mi) {
    /*
        Bu fonksiyon öncelikle dosyaya eklenecek olan structý oluþturuyor.
        i-n -> baþlangýç indeksi
        n -> bulunan benzerlik uzunluðu
        k -> dizi ise sýradaki harf
        dizi_mi -> benzerlik bulunup bulunmadýðý.

        Eðer benzerlik bulunmadýysa index  ve uzunluk 0 girilip harf structa
        eklenirken eðer benzerlik bulunduysa sýradaki harf structa eklenmektedir.
        Fwrite fonksiyonuyla binary formatýnda struct dosyaya yazýlmaktadýr.
    */
    if (!dizi_mi) {
        char str[10] = "";
        if (i<strlen(string)) {
            sprintf(str, "(%d,%d,%c)",0,0,string[i]);
        }
        else {
            sprintf(str, "(%d,%d,)",0,0);
        }
        sprintf(str, "(%d,%d,%c)",0,0,string[i]);
        fputs(str,file);
        return;
    }
    char str[] = "";
    if (k<strlen(string)) {
        sprintf(str, "(%d,%d,%c)",i-n,n,string[k]);
    }
    else {
        sprintf(str, "(%d,%d,)",i-n,n);
    }
    fputs(str,file);

}
void lz77_algoritmasi() {
    /*
        Bu fonksiyon öncelikle sýkýþtýrýlan yazýlarýn yazýlacaðý
        dosyayý binary olarak yazmak için açar. Sonra sýkýþtýrýlacak
        olan diziyi harf harf okuyarak arama tamponunda benzerlik arar.
        Maksimum 7 karakter öncesine bakarak aramayý son karaktere kadar
        sürdürür.

        Ýlk for döngüsü tüm karakterleri sýrayla geziyor. Ýkinci for döngüsü
        ilk for döngüsündeki indeks deðerinin bir öncesinden baþlayarak geriye
        doðru sýra ile tek tek benzerlik arýyor. 7 karakterden fazla uzaklaþýnca
        (i-j) ikinci döngüden çýkýyor ve benzerliði ekleyip sonraki harfe geçiyor.

        max -> en fazla benzerlik bulunan harf sayýsý
        index -> en fazla benzerlik bulunan benzerliðin baþlangýç indexi

        temp_max ve temp_index döngü içinde maksimum benzerliði tutan deðiþkenler
        temp_max deðeri mevcut maksimumdan büyük olduðunda maxa atanýyorlar.

        Eðer benzerlik bulunursa dizi_mi deðiþkeni 0'dan farklý bir deðer alýyor
        eðer benzerlik bulunmadýysa dizi_mi deðiþkeni 0 deðerini alarak lz77_yazdir
        fonksiyonuna gönderiliyor.

        Tüm döngüler bittiðinde dosyaya yazma iþlemi bittiðinden dosya kapatýlýyor.

    */
    file= fopen("lz77_output.txt", "w");
    int i, j, max, temp_max, index, temp_index, p, dizi_mi;
    for (i=0; i<strlen(string); i++) {
        dizi_mi = 0;
        for (j=i-1; j>-1; j--) {
            if (i-j>7) {
                break;
            }
            temp_max = 0, temp_index = j, p = 0;
            while (string[i+p] == string[j+p]) {
                p++;
                temp_max++;
                if (i+p>strlen(string) || j+p == i) {
                    break;
                }
            }
            if (temp_max > max) {
                max = temp_max, index = temp_index, dizi_mi = 1;
            }
        }
        if (dizi_mi) {
            i += max;
            lz77_yazdir(i-index, max, i, 1);
        }
        else {
            lz77_yazdir(i, 1, i, 0);
        }
        max = 0;
    }
    fclose(file);
}


/*
    Deflate Sýkýþtýrma Algoritmasý
*/
char binary_yerlestir[] = " ";


struct huffman {
    int kapasite;
    int id;
    struct huffman *sol;
    struct huffman *sag;
    struct deflate_frekans *harf;
    struct huffman *sonraki;
};
struct deflate_frekans {
    int index;
    int uzunluk;
    char kod[20];
    char harf;
    int frekans;
    struct deflate_frekans *sonraki;
};


struct huffman *den_ilk;
struct deflate_frekans *deflate_frekans_ilk;


void deflate_yazdir() {
    file= fopen("deflate_output.txt", "w");
    deflate_lz77_algoritmasi_yaz();
    fclose(file);
}
void huffmana_ekle(struct huffman *ekle) {
    if (den_ilk == NULL) {
        den_ilk = ekle;
        return;
    }
    struct huffman *ara = den_ilk;
    if (ekle->kapasite <= ara->kapasite) {
        ekle->sonraki = ara;
        den_ilk = ekle;
        return;
    }
    while (ara != NULL) {
        if (ara->sonraki==NULL) {
            ara->sonraki = ekle;
            return;
        }
        if (ekle->kapasite <= ara->sonraki->kapasite) {
            ekle->sonraki = ara->sonraki;
            ara->sonraki = ekle;
            return;
        }
        ara = ara->sonraki;
    }

}
void huffmandan_kaldir(struct huffman *sil) {
     struct huffman *ara = den_ilk;
     if (ara->id == sil->id) {
        den_ilk = ara->sonraki;
        return;
     }
     while (ara != NULL) {
        if (ara->sonraki->id==sil->id) {
            ara->sonraki = ara->sonraki->sonraki;
        }
        if (ara->sonraki == NULL) {
            break;
        }
        ara = ara->sonraki;
     }
}
void binary_kodlari_yerlestir(struct huffman *ara) {
    /*
        Recursive fonksiyon yardýmýyla huffman aðacýndaki deflate_frekans
        deðerine binary kodu yerleþtiriyor. Sol tarafa gittikçe binary_yerlestir
        stringine 1 sað tarafa gittikçe 0 deðerini koyuyor. Fonksiyon dönüþünde de
        o eklediðimiz 1 ya da 0 ý siliyor.
    */
    if (ara != NULL) {
        if (ara->harf != NULL) {
            strcpy(ara->harf->kod, binary_yerlestir);
        }
        if (ara->sol != NULL) {
            binary_yerlestir[strlen(binary_yerlestir)] = '1';
            binary_kodlari_yerlestir(ara->sol);
            binary_yerlestir[strlen(binary_yerlestir)-1] = '\0';
        }
        if (ara->sag != NULL) {
            binary_yerlestir[strlen(binary_yerlestir)] = '0';
            binary_kodlari_yerlestir(ara->sag);
            binary_yerlestir[strlen(binary_yerlestir)-1] = '\0';
        }
    }
}
void huffmani_olustur() {
    /*
        Harfleri sýrasýyla tekrar sýralarýna göre huffman aðacýna kaydediyor.
        Structýn ilk elemanýný alýyor ve huffmana ekleyeceði ekle adlý structýn
        soluna ekliyor. Sonraki döngüde ikinci elemaný alýyor ve eklenin structýnýn
        saðýna ekliyor. Daha sonra bu sola ve saða eklediði elemanlarý huffman aðacýndan
        silip onlarýn yerine ekle structýný ekliyor. Silme iþlemleri huffmandan_kaldir
        fonksiyonunda, ekleme iþlemi hufmanna_ekle fonksiyonunda gerçekleþiyor. Ekleme
        iþleminden sonra huffman aðacýnýn tekrar ilk elemanýna dönüp ilk iki eleman alýnýyor.
        Ta ki huffmanda tek eleman kalana kadar.

        Örnek:

        1 1 1 3 4
        1. adýmda ->
                    ekle->sol = 1
                    ekle = 1
        2. adýmda ->
                    ekle->sag = 1
                    ekle = 2
                    solu huffmandan kaldýr
                    saðý huffmandan kaldýr
                    hufmanýn son durumu: 1 3 4
                    ekleyi huffmana sýralamaya uygun olarak ekle
                    hufmanýn son durumu: 1 2 3 4
                    aðacýn baþýna dön (temp = den_ilk)
        3. adýmda ->
                    ekle->sol = 1
                    ekle = 1
        4. adýmda ->
                    ekle->sag = 2
                    ekle = 3
                    solu huffmandan kaldýr
                    saðý huffmandan kaldýr
                    hufmanýn son durumu: 3 4
                    ekleyi huffmana sýralamaya uygun olarak ekle
                    hufmanýn son durumu: 3 3 4
                    aðacýn baþýna dön (temp = den_ilk)

        *** Ekleme yaparken ayný deðerde baþka eleman var ise onun baþýna eklenir.
        Örneðin 3 3 3 4 þeklinde eleman varken diziye bir tane 4 eklenmesi gerektiðinde
        eklenecek olan 4 mevcut 4 ün önüne eklenir ve þu þekilde olur: (yýldýzlý olan eklenen)
        3 3 3 4* 4
    */
    struct huffman *temp = den_ilk;
    struct huffman *ekle = (struct huffman*)malloc(sizeof(struct huffman));
    ekle->sol= NULL;
    ekle->sag = NULL;
    ekle->sonraki = NULL;
    ekle->kapasite = 0;
    ekle->harf = NULL;
    int c = 0;
    while (temp != NULL) {
        if (ekle->sol==NULL) {
            ekle->sol = temp;
            c = temp->kapasite;
        }
        else if (ekle->sag == NULL) {
            ekle->sag = temp;
            c += temp->kapasite;
            ekle->kapasite = c;
            huffmandan_kaldir(ekle->sol);
            huffmandan_kaldir(ekle->sag);
            huffmana_ekle(ekle);
            ekle = (struct huffman*)malloc(sizeof(struct huffman));
            ekle->sol= NULL;
            ekle->sag = NULL;
            ekle->sonraki = NULL;
            ekle->harf = NULL;
            ekle->kapasite = 0;
            temp = den_ilk;
            continue;
        }
        if (temp->sonraki==NULL) {
            break;
        }
        temp = temp->sonraki;
    }
}
void karakterleri_ekle() {
    /*
        Frekans structýndaki tüm harf ve harf dizilerini
        hufmann structýna sýrasýyla ekler.
    */
    struct huffman *ekle = (struct huffman*)malloc(sizeof(struct huffman));
    struct huffman *temp = (struct huffman*)malloc(sizeof(struct huffman));
    struct deflate_frekans *ara = deflate_frekans_ilk;
    int i = 0;
    while (ara != NULL) {
        ekle = (struct huffman*)malloc(sizeof(struct huffman));
        ekle->harf = ara;
        ekle->kapasite = ara->frekans;
        ekle->sag = NULL;
        ekle->sol = NULL;
        ekle->sonraki = NULL;
        if (i==0) {
            temp = ekle;
            den_ilk = ekle;
        }
        else {
            temp->sonraki = ekle;
            temp = temp->sonraki;
        }

        if (ara->sonraki == NULL) {
            break;
        }
        ara = ara->sonraki;
        i++;
    }
}
void huffman_algoritmasi() {
    karakterleri_ekle();
    huffmani_olustur();
    binary_kodlari_yerlestir(den_ilk);
}
void deflate_frekans_arttir(int index, int uzunluk, char s) {
    /*
        Parametre olarak gelen harfin tekrar sayýsýný bir kez arttýrýr,
        eðer daha önce bu harf eklenmediyse yeni bir harf olarak structa
        ekler.
    */
    struct deflate_frekans *ara = deflate_frekans_ilk;
    while (ara != NULL) {
        if (ara->harf==s && ara->index==index && ara->uzunluk==uzunluk) {
            ara->frekans++;
            return;
        }
        if (ara->sonraki == NULL) {
            break;
        }
        ara = ara->sonraki;
    }
    struct deflate_frekans *ekle = (struct deflate_frekans*)malloc(sizeof(struct deflate_frekans));
    ekle->frekans = 1;
    ekle->harf = s;
    ekle->uzunluk = uzunluk;
    ekle->index = index;
    ekle->sonraki = NULL;
    if (ara != NULL) {
        ara->sonraki = ekle;
    }
    else {
        deflate_frekans_ilk = ekle;
    }
}
void deflate_lz77_yaz(int index, int uzunluk, char s) {
    /*
        Burada benzerlik bulunmayanlar deflate_frekans_arttir
        fonksiyonu çaðrýlarak bu fonksiyonda var olan tüm farklý harfler bir
        structa kaydediliyor. Eðer benzerlik bulunduysa o zaman
        deflate_lz77_tekrarlar fonksiyonu çaðrýlýp orada structa ekleniyor.
    */
    struct deflate_frekans *ara = deflate_frekans_ilk;
        while (ara != NULL) {
            if (ara->harf==s && ara->index==index && ara->uzunluk==uzunluk) {
                fputs(ara->kod, file);
                return;
            }
            if (ara->sonraki == NULL) {
                break;
            }
            ara = ara->sonraki;
        }
}
void deflate_lz77_algoritmasi() {
    /*
        lz77_algoritmasýnýn aynýsý sadece lz77_yazdir fonksiyonu
        yerine deflate_lz77_yaz fonksiyonu çaðrýlýyor.
    */
    int i, j, max, temp_max, index, temp_index, p, dizi_mi;
    for (i=0; i<strlen(string); i++) {
        dizi_mi = 0;
        for (j=i-1; j>-1; j--) {
            if (i-j>7) {
                break;
            }
            temp_max = 0, temp_index = j, p = 0;
            while (string[i+p] == string[j+p]) {
                p++;
                temp_max++;
                if (i+p>strlen(string) || j+p == i) {
                    break;
                }
            }
            if (temp_max > max) {
                max = temp_max, index = temp_index, dizi_mi = 1;
            }
        }
        if (dizi_mi) {
            i += max;
            deflate_frekans_arttir(i-index-max, max, string[i]);
        }
        else {
            deflate_frekans_arttir(0, 0, string[i]);
        }
        max = 0;
    }


}
void deflate_lz77_algoritmasi_yaz() {
    /*
        deflate_lz77_algoritmasi aynýsý sadece burada bulunanlar yazdýrýlmak üzere
        fonksiyona gönderiliyor.
    */
    int i, j, max, temp_max, index, temp_index, p, dizi_mi;
    for (i=0; i<strlen(string); i++) {
        dizi_mi = 0;
        for (j=i-1; j>-1; j--) {
            if (i-j>9) {
                break;
            }
            temp_max = 0, temp_index = j, p = 0;
            while (string[i+p] == string[j+p]) {
                p++;
                temp_max++;
                if (i+p>strlen(string) || j+p == i) {
                    break;
                }
            }
            if (temp_max > max) {
                max = temp_max, index = temp_index, dizi_mi = 1;
            }
        }
        if (dizi_mi) {
            i += max;
            deflate_lz77_yaz(i-index-max, max, string[i]);
        }
        else {
            deflate_lz77_yaz(0, 0, string[i]);
        }
        max = 0;
    }


}
void deflate_algoritmasi() {
    deflate_lz77_algoritmasi();
    huffman_algoritmasi();
    deflate_yazdir();
}


/*
    Main
*/


void inputu_oku() {
    binary_yerlestir[0] = '\0';
    file = fopen("input.txt", "r");
    char oku[255];
    if (file) {
        while (fgets(oku, 255, file)>0){
            sprintf(string, "%s%s",string,oku);
        }

        fclose(file);
    }
    else {
        puts("input.txt bulunamadi.");
        exit(0);
    }
}
int main()
{
    inputu_oku();
    lz77_algoritmasi();
    deflate_algoritmasi();
    return 0;
}
