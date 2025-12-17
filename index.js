const records= [{//skapar constat array för två objekt //varje objekt represeterar en användare 
    "id": 1, "username": "Mohammedkader", "medlem": 2000},
    {"id": 2, "username": "omar kadiiiir", "medlem":1987}
];


for (const u of records){//skriv ut varje array i records 
    console.log (u.medlem); //skriv värdet alla medlemar
}