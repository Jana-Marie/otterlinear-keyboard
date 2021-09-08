$fn = 100;
renderCap = false;
text1 = "Keycap laser alignment tool #1";
text2 = "Keycap laser alignment tool #2";
text3 = "Design by: Jana Marie Hemsing";
font = "Liberation Sans";
 
//%translate([0,50,0])color("DarkTurquoise")cylinder(d=2,h=10);

translate([0,-10*sin($t),0])holder();
aligner();

if(renderCap == true){
    for(i = [-1:2:1]){
        for(j = [-1:2:1]){
            %translate([10*i,10*j,3]) keycap();
        }
    }
}

module aligner(){
    color("Honeydew")
    difference(){
        union(){
            translate([0,40/2+15/2+0.1,2])
            minkowski(){
                cylinder(h=0.01,d=3);
                cube([42-3,15-3,4],center=true);
            }
        }
        union(){
            for(i = [-1:2:1]){
                translate([10*i,17.5+5,1.5])cube([2.1,5.1,3.1],center=true);
                translate([(42/2-4)*i,31,1])cylinder(d=6,h=4);
            }
            translate([0,26,3.8]) linear_extrude(height = 0.5) {
                text(text = str(text2), font = font, size = 2, valign = "center", halign = "center");
            }
            translate([0,22.5,3.8]) linear_extrude(height = 0.5) {
                text(text = str(text3), font = font, size = 2, valign = "center", halign = "center");
            }
        }
    }
}

module holder(){
    color("MintCream")
    difference(){
        union(){
            translate([0,0,2])
            minkowski(){
                cylinder(h=0.01,d=3);
                cube([42-3,40-3,4],center=true);
            }
            for(i = [-1:2:1]){
                translate([10*i,17.5+4.5,1.5])cube([2,4.5,3],center=true);
            }
        }
        union(){
            for(i = [-1:2:1]){
                for(j = [-1:2:1]){
                    translate([10*i,10*j,3]) keycap(0.2);
                    translate([10*i,10*j,-1]) pillars(0.2);
                }
            }
            translate([0,0,3.8]) linear_extrude(height = 0.5) {
                text(text = str(text1), font = font, size = 2, valign = "center", halign = "center");
            }
        }
    }
}

module pillars(,expand = 0,dia1 = 4){
    for(j = [-1:2:1]){
        hull(){
            for(i = [-1:2:1]){
                translate([((17.5-dia1+expand)/2)*i,((14.5-dia1+expand)/2)*j,0]) cylinder(d=dia1,h=5);
            }
        }
    }
}


// keycap
// todo legs
module keycap(expand = 0){
    color("DeepPink")
    difference(){
        union(){
            dia1 = 1;
            minkowski(){
                cylinder(h=3,d1=dia1,d2=1);
                cube([17.5-dia1+expand,16.5-dia1+expand,0.1],center=true);
            }
            for(i = [-1:2:1]){
                translate([(5.7/2)*i,0,-2]) cube([1.6,3.4,4],center=true);
            }
        }
        union(){
        }
    }
}