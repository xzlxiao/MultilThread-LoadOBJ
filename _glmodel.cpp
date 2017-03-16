#include<qfile.h>
#include<qdebug.h>

using namespace std;
#include<_glmodel.h>
#include<transform.h>
#include<gleasymath.h>

#ifndef GL_PI
#define GL_PI 3.14159265358979323846
#endif

enum{ _X, _Y, _Z, _W };//

#pragma region 私有方法
//根据文件的路径获取到文件夹的路径
QString _glGetDir(QString filePath)
{
	int index = filePath.lastIndexOf('\\');
	if (index == -1)
		index = filePath.lastIndexOf('/');
	filePath.remove(index + 1, filePath.length() - index - 1);
	return filePath;
}

//读取mtl文件
void _glReadMTL(_GLModel *model, QString fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug("mtl文件打开失败。");
		return;
	}
	QString dirPath;

	Material *material = NULL; QStringList list;
	int index = -1;//材质索引
	QString split = ' ';
	while (!file.atEnd())
	{
		QByteArray line = file.readLine();
		if (line.length() == 2 && line.at(line.length() - 2) == '\r'&&line.at(line.length() - 1) == '\n')//如果读到了空行且材质指针不为空，那么证明当前的材质已经读完
		{
			if (material&&material->materialName != NULL)
				model->list_Materials.push_back(*material);
		}

		QString str(line);
		if (str[0] == 'n')//名称
		{
			list = str.split(split);
			material = new Material();
			material->_Ka[_X] = 0.0; material->_Ka[_Y] = 0.0; material->_Ka[_Z] = 0.0;
			material->_Kd[_X] = 0.0; material->_Kd[_Y] = 0.0; material->_Kd[_Z] = 0.0;
			material->_Ks[_X] = 0.0; material->_Ks[_Y] = 0.0; material->_Ks[_Z] = 0.0;

			QString str1 = list[1];
			material->materialName = str1.trimmed();
			material->index_Material = ++index;
			model->num_Materials++;
		}
		else if (str[0] == 'm')//贴图路径
		{
			list = str.split(split);
			dirPath = _glGetDir(fileName);//获取文件夹路径
			dirPath.append(list[1].trimmed());
			material->imageName = dirPath;
			model->list_ImagePath.push_back(dirPath);
		}
		else if (str[0] == 'K')
		{
			list = str.split(split);
			if (str[1] == 'a')
			{
				material->_Ka[0] = list[1].toFloat();
				material->_Ka[1] = list[2].toFloat();
				material->_Ka[2] = list[3].toFloat();
			}
			else if (str[1] == 'd')
			{
				material->_Kd[0] = list[1].toFloat();
				material->_Kd[1] = list[2].toFloat();
				material->_Kd[2] = list[3].toFloat();
			}
			else if (str[1] == 's')
			{
				material->_Ks[0] = list[1].toFloat();
				material->_Ks[1] = list[2].toFloat();
				material->_Ks[2] = list[3].toFloat();
			}
		}
	}
}

#pragma endregion

//读取OBJ文件
_GLModel* _glReadOBJ(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return NULL;
	}
	QString dirPath = _glGetDir(filename); QStringList list; QString currentMaterialName;

	_GLModel* model;
	model = new _GLModel();
	QString split = ' ';

	model->path = filename;
	model->num_Faces = 0;
	model->num_Materials = 0;
	model->num_Normals = 0;
	model->num_Textcoords = 0;
	model->num_Vertices = 0;

	Point3 *v;
	TextCoords *vt;
	VertNormals *vn;
	Face *f;

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QString str(line);
		if (str.length() < 2)//太短~
			continue;
		if (str[0] == 'm')
		{
			QStringList str0 = str.split(' ');
			QString mtlname = str0[1];
			mtlname = mtlname.trimmed();
			dirPath.append(mtlname);
			model->mtllibName = dirPath;
			_glReadMTL(model, model->mtllibName);
		}
		else if (str[0] == 'v'){
			if (str[1] == 't'){//纹理
				list = str.split(split);//无论是否包括Z方向的纹理都先取前两个值
				vt = new TextCoords();
				vt->U = list[_Y].toFloat(); vt->V = list[_Z].toFloat();
				model->num_Textcoords++;
				model->list_Textcoords.push_back(*vt);
			}
			else if (str[1] == 'n'){//法向量
				list = str.split(split);
				vn = new VertNormals();
				vn->_NX = list[_Y].toFloat(); vn->_NY = list[_Z].toFloat(); vn->_NZ = list[_W].toFloat();
				model->num_Normals++;
				model->list_Normals.push_back(*vn);
			}
			else//节点~
			{
				list = str.split(split);
				v = new Point3();
				v->_X = list[_Y].toFloat(); v->_Y = list[_Z].toFloat(); v->_Z = list[_W].toFloat();
				model->num_Vertices++;
				model->list_Vertices.push_back(*v);
			}
		}
		else if (str[0] == 'u')//材质的名称
		{
			list = str.split(split);
			currentMaterialName = list[1];
		}
		else if (str[0] == 'f')//面
		{
			str = str.trimmed();
			list = str.split(split);
			
			f = new Face();
			f->materialName = currentMaterialName;

			if (list[1].contains('/'))
			{

				for (int i = 1; i < list.length(); i++)
				{
					QStringList sublist = list[i].split('/');
					f->list_index_Points.push_back(sublist[_X].toInt() - 1);
					f->list_index_TextCoords.push_back(sublist[_Y].toInt() - 1);
					if (list[1].split('/').length() == 3)//只有v和vt
					{
						f->list_index_VertNormals.push_back(sublist[_Z].toInt() - 1);
					}
				}
			}
			else//不包括/，那么只有节点
			{
				for (int i = 1; i < list.length(); i++)
				{
					f->list_index_Points.push_back(list[i].toInt() - 1);
				}
			}
			model->num_Faces++;
			model->list_Faces.push_back(*f);
		}
	}
	return model;
}

//释放之前的model
void _glDelete(_GLModel* model)
{
	if (model)
		delete model;
	//释放model中的集合对象？
}
//计算面的法向量

void _glFacetNormals(_GLModel* model)
{
	FacetNormal *fn;
	float u[3];
	float v[3];

	float cross[3];

	for (int i = 0; i < model->list_Faces.length(); i++)
	{
		fn = new FacetNormal();
		Point3 p0 = model->list_Vertices[model->list_Faces[i].list_index_Points[0]];
		Point3 p1 = model->list_Vertices[model->list_Faces[i].list_index_Points[1]];
		//Point3 p2 = model->list_Vertices[model->list_Faces[i].list_index_Points[2]];

		Point3 pn = model->list_Vertices[model->list_Faces[i].list_index_Points[model->list_Faces[i].list_index_Points.length() - 1]];//必须使用最后一点才成功

		u[_X] = p1._X - p0._X;
		u[_Y] = p1._Y - p0._Y;
		u[_Z] = p1._Z - p0._Z;

		//v[_X] = p2._X - p0._X;
		//v[_Y] = p2._Y - p0._Y;
		//v[_Z] = p2._Z - p0._Z;

		v[_X] = pn._X - p0._X;
		v[_Y] = pn._Y - p0._Y;
		v[_Z] = pn._Z - p0._Z;

		vCross(u, v, cross);
		vNormal(cross);

		model->list_Faces[i].index_Face = i;
		fn->NX = cross[0];
		fn->NY = cross[1];
		fn->NZ = cross[2];
		model->list_FaceNormal.push_back(*fn);
	}
}

//将图形移到屏幕中间来
float _glUnitize(_GLModel* model)
{
	float maxx, minx, maxy, miny, maxz, minz;
	float cx, cy, cz, w, h, d;
	float scale;

	if (model&&model->list_Vertices.size() > 0)
	{
		maxx = minx = model->list_Vertices[0]._X;
		maxy = miny = model->list_Vertices[0]._Y;
		maxz = minz = model->list_Vertices[0]._Z;

		for (size_t i = 1; i < model->num_Vertices; i++)
		{
			if (maxx < model->list_Vertices[i]._X)
				maxx = model->list_Vertices[i]._X;
			if (minx > model->list_Vertices[i]._X)
				minx = model->list_Vertices[i]._X;


			if (maxy < model->list_Vertices[i]._Y)
				maxy = model->list_Vertices[i]._Y;
			if (miny > model->list_Vertices[i]._Y)
				miny = model->list_Vertices[i]._Y;


			if (maxz < model->list_Vertices[i]._Z)
				maxz = model->list_Vertices[i]._Z;
			if (minz > model->list_Vertices[i]._Z)
				minz = model->list_Vertices[i]._Z;
		}

		w = _glmAbs(maxx) + _glmAbs(minx);
		h = _glmAbs(maxy) + _glmAbs(miny);
		d = _glmAbs(maxz) + _glmAbs(minz);

		//计算模型的中心
		cx = (maxx + minx) / 2.0;
		cy = (maxy + miny) / 2.0;
		cz = (maxz + minz) / 2.0;

		scale = 2.0 / _glmMax(w, _glmMax(h, d));
		//scale = _glmMax(w, _glmMax(h, d));

		//将中心按照比例转换
		for (size_t i = 0; i < model->num_Vertices; i++)
		{
			model->list_Vertices[i]._X -= cx;
			model->list_Vertices[i]._Y -= cy;
			model->list_Vertices[i]._Z -= cz;

			model->list_Vertices[i]._X *= scale;
			model->list_Vertices[i]._Y *= scale;
			model->list_Vertices[i]._Z *= scale;
		}
	}
	return scale;
}

void _glConstructIndexFromName(_GLModel* model)
{
	for (size_t i = 0; i < model->num_Faces; i++)
	{
		QString name = model->list_Faces[i].materialName;
		int index = name.toInt() - 1;
		model->list_Faces[i].index_Text = index;
	}
}

//渲染模型
void _glDraw(_GLModel* model, size_t mode)
{
	if (mode & _GL_FLAT && model->list_FaceNormal.size() == 0)
	{
		qDebug(T_QString2Char("Flat模式不可用！"));
		mode &= ~_GL_FLAT;
	}
	if (mode & _GL_SMOOTH && model->num_Normals == 0) {
		qDebug(T_QString2Char("Smooth模式不可用！"));
		mode &= ~_GL_SMOOTH;
	}
	if (mode & _GL_TEXTURE && model->num_Textcoords == 0) {
		qDebug(T_QString2Char("Texture模式不可用！"));
		mode &= ~_GL_TEXTURE;
	}
	glPushMatrix();
	//glTranslatef(model->Center[0], model->Center[1], model->Center[2]);

	for (size_t i = 0; i < model->num_Faces; i++)
	{
		Face f = model->list_Faces[i];
		if (mode&_GL_TEXTURE)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, model->textureArray[f.index_Text]);
		}
		glBegin(GL_POLYGON);
		//glBegin(GL_QUADS);
		if (mode&_GL_FLAT)
			glNormal3f(model->list_FaceNormal[f.index_Face].NX, model->list_FaceNormal[f.index_Face].NY, model->list_FaceNormal[f.index_Face].NZ);

		for (int k = 0; k < f.list_index_Points.size(); k++)
		{
			if (mode&_GL_TEXTURE)
			{
				glTexCoord2f(model->list_Textcoords[f.list_index_TextCoords[k]].U, model->list_Textcoords[f.list_index_TextCoords[k]].V);
			}
			if (mode&_GL_SMOOTH&&f.list_index_VertNormals.size()>0)
			{
				glNormal3f(model->list_Normals[f.list_index_VertNormals[k]]._NX, model->list_Normals[f.list_index_VertNormals[k]]._NY, model->list_Normals[f.list_index_VertNormals[k]]._NZ);
			}
			glVertex3f(model->list_Vertices[f.list_index_Points[k]]._X, model->list_Vertices[f.list_index_Points[k]]._Y, model->list_Vertices[f.list_index_Points[k]]._Z);
		}
		glEnd();
	}
	glPopMatrix();

}